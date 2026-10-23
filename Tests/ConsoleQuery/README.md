# Console and player query regression tests

Unit tests can run without server dependencies or submodules:

```sh
cmake -S Tests/ConsoleQuery -B build/regression -DCMAKE_BUILD_TYPE=Release
cmake --build build/regression --config Release
ctest --test-dir build/regression -C Release --output-on-failure
```

The packet tests run on all supported C++17 platforms. Windows additionally tests
lossless GBK, Big5 and UTF-8 conversion, invalid Unicode and unsupported characters.

For the full Windows integration test, configure the normal server build from
[BUILD.md](../../BUILD.md) with these additional options (Python 3 is required):

```sh
-DBUILD_CONSOLE_QUERY_TESTS=ON -DBUILD_CONSOLE_QUERY_FIXTURES=ON
```

Build the project, then run `ctest --test-dir build -C RelWithDebInfo --output-on-failure`.
Adjust the directory and configuration to match your build. Fixtures are written
under `Tests/ConsoleQuery/fixtures` in the build directory, outside the server
output. **Never deploy these test components.**

`integration.py` starts isolated instances on ephemeral loopback UDP ports with
fresh random passwords, no user configuration and no game mode. It verifies
0/1/1000 players, actual player pool IDs, retained snapshot generations, malformed
requests, old `c`/`d` layouts, disabled Query/RCON, real Windows UTF-16 stdin,
outgoing game RPC bytes, Unicode passwords, and rejection without dispatch.
Temporary files and child processes are cleaned up on success and failure.

This verifies the server's network bytes, not rendering in a real game client.
