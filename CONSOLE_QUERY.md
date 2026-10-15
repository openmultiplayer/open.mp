# Legacy console text and detailed player queries

These changes are source changes proposed for review, not an official open.mp
release. Build the whole server from one revision; do not mix components from
this checkout with a different server/SDK revision.

## Windows console and remote RCON encoding

The game protocol sends text bytes without an encoding marker. For a GBK game
server, Unicode text entered at the Windows console must become GBK before it
reaches `say` or a script command. Add this to the existing `config.json`:

```json
"console": { "input_codepage": 936 }
```

`console.input_codepage` defaults to **65001 (UTF-8)**, preserving existing
behavior. Use 936 for GBK, 950 for Big5, or 0 for the Windows system ANSI code
page. Conversion rejects invalid or unrepresentable characters, including emoji
that GBK cannot encode, instead of executing a changed command. Local command
replies are converted back to UTF-8 for console/log output. Settings are Windows
only; POSIX console behavior is unchanged.

Remote RCON defaults to forwarding the original command bytes. Clients that
already send GBK, including SampCtrl's AUTO/GBK mode, need no server conversion.
For a client that sends **UTF-8 commands** to a GBK game server, explicitly enable:

```json
"rcon": { "utf8_command_codepage": 936 }
```

Merge this property into the existing `rcon` object, preserving its `enable` and
`password` properties. Zero (the default) disables conversion. A nonzero value
selects the Windows code page used for the decoded UTF-8 command. Malformed UTF-8
and lossy conversions return an error and do not dispatch the command. Passwords
are checked against their original bytes before any conversion; replies retain
their original byte encoding. In-game RCON and `SendRconCommand` are unchanged.

All remote senders must agree on UTF-8 before enabling this option. Do not enable
it while SampCtrl is in AUTO/GBK mode: either leave it at 0 or select UTF-8 in the
app. UTF-8 command input does not change the encoding of server replies, player
names, configuration strings, or other game messages. A client's response decoder
must still support the server's reply encoding. There is no reliable automatic
way to distinguish untagged UTF-8 from every legacy encoding.

## Public detailed player queries

The existing `c` response remains unchanged: it has names and scores but no IDs.
The legacy `d` response supplies real pool IDs, names, scores and ping when there
are at most 100 players and every ID fits in a byte. If any ID exceeds 255, `d`
does not reply; it never truncates an ID or substitutes the row number.

The proposed uppercase `D` extension supplies 16-bit IDs in bounded pages. It
uses the existing SampCtrl `sc_d16=1` rule as a compatibility capability marker.
This is an experimental extension for upstream review, not a claim of an
official standardized protocol. It exposes the same public names/scores plus
IDs and ping; it contains no IP addresses or credentials. `enable_query=false`
disables it together with the existing queries. RCON authentication is neither
required for reading this public information nor bypassed for management actions.

All integers are little-endian. Requests start with the normal 11-byte SAMP query
header with opcode `D`, followed by:

| Field | Bytes | Meaning |
| --- | ---: | --- |
| version | 1 | 1 |
| generation | 4 | 0 for the first page; reuse the returned generation thereafter |
| page | 2 | Zero based; generation 0 requires page 0 |

Responses echo the query header, then version(1), generation(4), total(2), page(2),
pageCount(2), count(2). Each record contains ID(2), nameLength(1), name bytes,
signed score(4), unsigned ping(4). Name bytes are unchanged, at most 24 bytes.

- The main thread snapshots non-bot players every second, sorted by actual ID.
- Each page has at most 32 players. At 1000 players there are 32 pages; even with
  maximum-length names a response is at most 1144 bytes, including the query header.
- An empty server returns one empty page. Generation 0 is never emitted.
- The last 16 snapshots are retained, bounding memory and supporting consistent
  pagination while players change. Bad lengths, versions, pages and expired
  generations receive no reply. A client should restart at generation 0 after
  an unavailable page and reject mixed generations or duplicate IDs.
- A snapshot may be up to one refresh interval old. Management clients must
  revalidate identity through authenticated commands before acting on an ID
  that may have been reused after a disconnect.

See [the regression test instructions](Tests/ConsoleQuery/README.md) for local
unit tests and isolated server integration tests.
