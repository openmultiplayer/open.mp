"""Exercise the built server over loopback and capture real outgoing message RPCs.

Only the two test components are loaded, never a user's game mode or configuration.
"""
import argparse
import json
import os
from pathlib import Path
import queue
import secrets
import shutil
import socket
import struct
import subprocess
import tempfile
import threading
import time


def check(condition, message):
    if not condition:
        raise AssertionError(message)


class Server:
    def __init__(self, args, fixture, count=1, query_enabled=True, codepage=None, remote=None):
        self.args, self.fixture = args, fixture
        self.count, self.query_enabled = count, query_enabled
        self.codepage, self.remote = codepage, remote
        self.password = "密码_" + secrets.token_hex(12)
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
            sock.bind(("127.0.0.1", 0))
            self.port = sock.getsockname()[1]
        self.lines, self.pending = [], queue.Queue()

    def __enter__(self):
        self.temp = tempfile.TemporaryDirectory(prefix="omp-console-query-")
        root = Path(self.temp.name)
        components = root / "components"
        components.mkdir()
        shutil.copy2(self.args.server, root / "omp-server.exe")
        for path in (self.args.network, self.args.console, self.fixture):
            shutil.copy2(path, components / Path(path).name)
        config = {
            "name": "Isolated console/query regression", "announce": False,
            "enable_query": self.query_enabled, "max_players": max(1, self.count),
            "network": {"bind": "127.0.0.1", "port": self.port},
            "rcon": {"enable": self.fixture == self.args.console_fixture, "password": self.password},
            "logging": {"enable": True, "file": "test.log"},
        }
        if self.codepage is not None:
            config["console"] = {"input_codepage": self.codepage}
        if self.remote is not None:
            config["rcon"]["utf8_command_codepage"] = self.remote
        (root / "config.json").write_text(json.dumps(config), encoding="utf-8")
        env = dict(os.environ, SAMPCTRL_TEST_PLAYERS=str(self.count))
        self.process = subprocess.Popen(
            [str(root / "omp-server.exe")], cwd=root, env=env,
            stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
            creationflags=subprocess.CREATE_NO_WINDOW,
        )
        self.reader = threading.Thread(target=self.read_lines, daemon=True)
        self.reader.start()
        try:
            marker = "[SC-QUERY-TEST] READY" if self.fixture == self.args.query_fixture else "[SC-CONSOLE-TEST] READY"
            self.wait_line(marker)
        except BaseException:
            self.__exit__(None, None, None)
            raise
        return self

    def read_lines(self):
        for raw in self.process.stdout:
            line = raw.decode("utf-8", errors="replace").rstrip()
            self.lines.append(line)
            self.pending.put(line)

    def wait_line(self, marker, timeout=10):
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            try:
                line = self.pending.get(timeout=min(.2, max(.01, deadline - time.monotonic())))
            except queue.Empty:
                if self.process.poll() is not None:
                    break
                continue
            if marker in line:
                return line
        raise AssertionError(f"Missing {marker!r}:\n" + "\n".join(self.lines[-30:]))

    def local(self, command):
        self.process.stdin.write((command + "\n").encode("utf-16-le"))
        self.process.stdin.flush()

    def packet(self, opcode, payload=b"", expect=True):
        header = b"SAMP" + socket.inet_aton("127.0.0.1") + struct.pack("<H", self.port) + opcode
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
            sock.settimeout(.3)
            sock.connect(("127.0.0.1", self.port))
            sock.send(header + payload)
            try:
                response = sock.recv(8192)
            except socket.timeout:
                if not expect:
                    return None
                raise AssertionError(f"No reply to {opcode!r}")
        check(expect, f"Unexpected reply to {opcode!r}")
        check(response[:11] == header, "Query header mismatch")
        return response

    def rcon(self, command, password=None, reply=True):
        password = (self.password if password is None else password).encode("utf-8")
        payload = struct.pack("<H", len(password)) + password + struct.pack("<H", len(command)) + command
        response = self.packet(b"x", payload, expect=reply)
        if response is None:
            return None
        length, = struct.unpack_from("<H", response, 11)
        check(len(response) == 13 + length, "Bad RCON response length")
        return response[13:]

    def page(self, generation=0, page=0):
        return self.packet(b"D", struct.pack("<BIH", 1, generation, page))

    def __exit__(self, *_):
        if self.process.poll() is None:
            self.process.kill()
        self.process.wait(timeout=5)
        self.reader.join(timeout=3)
        for stream in (self.process.stdin, self.process.stdout):
            stream.close()
        self.temp.cleanup()


def parse_page(packet):
    version, generation, total, page, pages, count = struct.unpack_from("<BIHHHH", packet, 11)
    check(version == 1 and generation != 0 and len(packet) <= 1144, "Invalid page header")
    records, offset = [], 24
    for _ in range(count):
        player_id, length = struct.unpack_from("<HB", packet, offset)
        name = packet[offset + 3:offset + 3 + length]
        score, ping = struct.unpack_from("<iI", packet, offset + 3 + length)
        records.append((player_id, name, score, ping))
        offset += 11 + length
    check(offset == len(packet), "Trailing or truncated page data")
    return generation, total, page, pages, records


def query_tests(args):
    for count in (0, 1, 1000):
        with Server(args, args.query_fixture, count) as server:
            deadline = time.monotonic() + 5
            while True:
                first = parse_page(server.page())
                if first[1] == count:
                    break
                check(time.monotonic() < deadline, "Snapshot did not update")
                time.sleep(.05)
            generation, total, _, pages, records = first
            records = list(records)
            check(pages == max(1, (total + 31) // 32), "Wrong page count")
            for page in range(1, pages):
                current = parse_page(server.page(generation, page))
                check(current[:4] == (generation, total, page, pages), "Mixed generations")
                records += current[4]
            check([p[0] for p in records] == list(range(count)), "Player IDs changed or disappeared")
            for pid, name, score, _ in records:
                check(name == f"SC_Test_{pid}".encode() and score == 10000 + pid, "Wrong player record")
            rules = server.packet(b"r")
            check(b"\x06sc_d16\x011" in rules, "Capability not advertised")
            legacy = server.packet(b"d", expect=count <= 100)
            basic = server.packet(b"c", expect=count <= 100)
            if count <= 100:
                check(struct.unpack_from("<H", legacy, 11)[0] == count, "Legacy count")
                check(struct.unpack_from("<H", basic, 11)[0] == count, "Basic query changed")
                if count:
                    check(legacy[13] == 0 and basic[13] == len(b"SC_Test_0"), "Legacy/basic layout changed")
            for malformed in (b"", b"\x01", struct.pack("<BIH", 2, 0, 0),
                              struct.pack("<BIH", 1, 0, 1), struct.pack("<BIH", 1, generation, pages),
                              struct.pack("<BIH", 1, 0xffffffff, 0), struct.pack("<BIH", 1, 0, 0) + b"x"):
                server.packet(b"D", malformed, expect=False)
            server.packet(b"d", b"x", expect=False)
            # A later snapshot must not replace pages retained for an existing reader.
            newer = parse_page(server.page())
            check(newer[0] != generation, "Snapshot generation did not advance")
            old = parse_page(server.page(generation))
            check(old[:4] == first[:4] and old[4] == first[4], "Retained snapshot changed")
            password = server.password.encode()
            server.packet(b"x", struct.pack("<H", len(password)) + password + b"\x04\x00echo", expect=False)
        print(f"PASS query: {count} players, IDs, pagination, legacy layout, retained snapshots, malformed requests, RCON disabled", flush=True)
    with Server(args, args.query_fixture, query_enabled=False) as server:
        for opcode, payload in ((b"D", struct.pack("<BIH", 1, 0, 0)), (b"d", b""), (b"c", b""), (b"r", b"")):
            server.packet(opcode, payload, expect=False)
    print("PASS query: enable_query=false suppresses all player queries", flush=True)


def console_tests(args):
    prefix = b"* Admin: "
    for codepage, encoding in ((None, "utf-8"), (936, "gbk"), (950, "big5")):
        with Server(args, args.console_fixture, codepage=codepage) as server:
            expected = "中文".encode(encoding)
            server.local("say 中文")
            server.wait_line("RPC " + (prefix + expected).hex().upper())
            server.local("sc_test_command 中文")
            server.wait_line("CUSTOM " + expected.hex().upper())
            server.local("echo local_echo_中文")
            server.wait_line("local_echo_中文")
            server.local("sc_test_script")
            server.wait_line("RPC " + (prefix + "中文".encode("gbk")).hex().upper())
            for remote_encoding in ("gbk", "utf-8"):
                command = "say 中文".encode(remote_encoding)
                server.rcon(command, reply=False)
                server.wait_line("RPC " + (prefix + command[4:]).hex().upper())
            if codepage in (936, 950):
                server.local("say 😀")
                server.wait_line("Local console command was not executed")
            server.local("say ASCII_123")
            server.wait_line("RPC " + (prefix + b"ASCII_123").hex().upper())
            check(sum("[SC-CONSOLE-TEST] RPC " in line for line in server.lines) == 5, "Unexpected or duplicate dispatch")
        print(f"PASS console: codepage={codepage}, real Unicode stdin, exact RPC bytes, echo, raw script API, unchanged GBK/UTF-8 RCON", flush=True)
    with Server(args, args.console_fixture, codepage=936, remote=936) as server:
        server.rcon("say 中文 ABC 123".encode(), reply=False)
        server.wait_line("RPC " + (prefix + "中文 ABC 123".encode("gbk")).hex().upper())
        check(server.rcon("echo 中文".encode()) == "中文".encode("gbk"), "Remote echo encoding")
        for command in ("say 😀".encode(), "say 中文".encode("gbk"), b"say \xc0\xaf"):
            check(b"was not executed" in server.rcon(command), "Invalid command was accepted")
        check(b"Invalid RCON password" in server.rcon(b"say NO", password="incorrect"), "Authentication changed")
        server.local("sc_test_script")
        server.wait_line("RPC " + (prefix + "中文".encode("gbk")).hex().upper())
        server.rcon(b"say ASCII", reply=False)
        server.wait_line("RPC " + (prefix + b"ASCII").hex().upper())
        check(sum("[SC-CONSOLE-TEST] RPC " in line for line in server.lines) == 3, "Rejected command dispatched")
    print("PASS RCON: opt-in UTF-8 to GBK, Unicode password, exact RPC, raw replies/API, rejection without dispatch", flush=True)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    for name in ("server", "network", "console", "query-fixture", "console-fixture"):
        parser.add_argument("--" + name, required=True)
    args = parser.parse_args()
    query_tests(args)
    console_tests(args)
