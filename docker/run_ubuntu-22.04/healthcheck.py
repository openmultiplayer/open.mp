#!/usr/bin/env python3

import socket
import sys
import os

sock = None

try:
    # Read environment variables in case they are passed
    host = os.getenv("SAMP_HOST", "127.0.0.1")
    port = int(os.getenv("SAMP_PORT", "7777"))
    timeout = float(os.getenv("SAMP_QUERY_TIMEOUT", "2"))
    encoding = os.getenv("SAMP_QUERY_ENCODING", "cp1251")
    query_buffer = int(os.getenv("SAMP_QUERY_BUFFER", "256")) # Buffer to fit the response, including very long hostnames and gamemodes

    # Create a packet based on the guide: https://open.mp/docs/tutorials/QueryMechanism
    packet = (b"SAMP" + socket.inet_aton(host) + bytes([port & 0xFF]) + bytes([(port >> 8) & 0xFF]) + b"i")

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(timeout) 
    sock.sendto(packet, (host, port))
    response, _ = sock.recvfrom(query_buffer)

    # Make sure the response contains at least the hostname (byte 20)
    if len(response) < 20:
        raise ValueError("Response is too short")

    if response[:11] != packet[:11]:
        raise ValueError("Invalid response header")

    # Protocol byte 12 is players (2B)
    players = int.from_bytes(response[12:14], "little")

    # Protocol byte 14 is max players (2B)
    max_players = int.from_bytes(response[14:16], "little")

    # Protocol byte 16 is hostname length (4B)
    hostname_length = int.from_bytes(response[16:20], "little")

    # Protocol byte 20 is hostname (hostname_length bytes long)
    hostname_raw = response[20:20 + hostname_length]
    
    # Check if the extracted hostname is actually the same length
    if len(hostname_raw) != hostname_length:
        raise ValueError("Response is too short for hostname")

    hostname = hostname_raw.decode(encoding)

    # Protocol byte 20+x is gamemode length (4B)
    gamemode_length_start = 20 + hostname_length

    # Make sure the response contains the gamemode length in case the response is truncated
    if len(response) < gamemode_length_start + 4:
        raise ValueError("Response is too short for gamemode length")

    gamemode_length = int.from_bytes(response[gamemode_length_start:gamemode_length_start + 4], "little")

    # Protocol byte 20+x+4 is gamemode (gamemode_length bytes long)
    gamemode_start = gamemode_length_start + 4
    gamemode_raw = response[gamemode_start:gamemode_start + gamemode_length]

    # Check if the extracted gamemode is actually the same length in case the response is truncated
    if len(gamemode_raw) != gamemode_length:
        raise ValueError("Response is too short for gamemode")

    gamemode = gamemode_raw.decode(encoding)

    # If gamemode is 'Unknown', then something has not loaded properly
    if gamemode == "Unknown":
        raise ValueError("Gamemode is not running")

    print(f"OK: {players}/{max_players}, {hostname}, {gamemode}")

except Exception as error:
    print(f"FAIL: {error}")
    sys.exit(1)
finally:
    # Only close the socket if it exists
    if sock is not None:
        sock.close()