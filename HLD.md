# mini-Redis — High-Level Design

> Read this after `OVERVIEW.md` if you want the "how it actually works" level of detail —
> request lifecycle, concurrency model, persistence semantics, and the gaps that exist today.
> Written so a new contributor, or the original author returning after months away, can
> rebuild a working mental model from this file and the code, without re-deriving it from git history.

---

## 1. What This System Is

A single-process, single-port, in-memory key-value store that speaks a subset of RESP
(the Redis Serialization Protocol) over raw TCP. One `mini-redis` process listens on
`0.0.0.0:6379`, accepts many concurrent client connections (one OS thread per connection),
and serves `GET`/`SET`/`DEL`/`LPUSH`/`HSET`/`SUBSCRIBE`/`PUBLISH`/`UNSUBSCRIBE`/`SAVE`/`AUTH`
against a single shared, mutex-protected hash map held in process memory.

There is no clustering, no replication, and no network protocol negotiation — a client either
speaks RESP correctly or gets whatever the parser's best-effort tokenizing produces (see
[§8](#8-known-limitations) for where that bites).

---

## 2. Component Map

| Component | File(s) | Responsibility |
|---|---|---|
| `TcpServer` | `src/server/TcpServer.*` | Owns the listening socket; accepts connections; spawns one detached `std::thread` per client; owns the `CommandDispatcher` instance and registers every command's factory lambda; drives startup persistence loading |
| `RespParser` | `src/protocol/RespParser.*` | Turns raw bytes read off a socket into a `vector<string>` of tokens |
| `AuthProxy` | `src/auth/AuthProxy.*` | Singleton. Sits between `TcpServer` and `CommandDispatcher`; gates every command except `AUTH` behind per-connection authentication |
| `CommandDispatcher` | `src/commands/CommandDispatcher.*` | Looks up a token's command name in a factory map, builds the concrete `ICommand`, calls `execute()`, and AOF-logs the call if it's a registered write command |
| `ICommand` + `*Command` | `src/commands/*` | One class per verb (`GetCommand`, `SetCommand`, `AuthCommand`, ...); each `execute()` talks to `StoreEngine`, `PubSubManager`, `AuthProxy`, or `SnapshotWriter` as needed and returns a raw RESP reply string |
| `StoreEngine` | `src/store/StoreEngine.*` | Singleton. Owns the actual `unordered_map<string, DataEntry>`; every access is mutex-protected; also owns the active `IEvictionPolicy` and notifies it on get/set |
| `DataEntry` | `src/store/DataEntry.*` | Decorator wrapping `unique_ptr<IDataType>` with an absolute expiry timestamp (`-1` = no TTL) |
| `IDataType` + `StringType`/`ListType`/`HashType`/`SetType` | `src/store/types/*` | The four value types. Each knows its own `getType()` and a (lossy — see §8) string `serialize()` |
| `DataTypeFactory` | `src/store/types/DataTypeFactory.*` | Factory-pattern demo: builds an empty `IDataType` by type name. **Not currently called by any command or by `SnapshotWriter`** — those construct concrete types directly. Kept as the Factory pattern's canonical example per the project's design-patterns goal; see §7 |
| `IEvictionPolicy` + `LRUPolicy`/`LFUPolicy` | `src/eviction/*` | Strategy pattern. Track recency/frequency on every get/set. **`evict()` is never called by any command path today** — see §6 |
| `PubSubManager` | `src/pubsub/PubSubManager.*` | Singleton. `unordered_map<channel, vector<clientFd>>`; `publish()` writes RESP directly to each subscriber's raw socket fd from whatever thread called `publish()` |
| `AofWriter` | `src/persistence/AofWriter.*` | Singleton. Appends every successful write command to `appendonly.aof` as whitespace-joined tokens |
| `SnapshotWriter` | `src/persistence/SnapshotWriter.*` | Static `save()`/`load()`. Binary-serializes/deserializes the whole store to/from `dump.rdb` |

---

## 3. Startup Sequence

```
main()
  1. StoreEngine::getInstance().setEvictionPolicy(LRUPolicy)   // LRU is hardcoded as active policy
  2. if argv[1] given: AuthProxy::getInstance().setPassword(argv[1])
  3. construct TcpServer("0.0.0.0", 6379)
       → registers every command's factory lambda into CommandDispatcher (SET/GET/DEL/LPUSH/
         HSET/SUBSCRIBE/UNSUBSCRIBE/PUBLISH/SAVE/AUTH)
  4. server.loadSnapshot("dump.rdb")
       → if dump.rdb exists: binary-restore the whole store, return true
       → else: return false
  5. if step 4 returned false: server.loadAof("appendonly.aof")
       → replay every line in appendonly.aof through the dispatcher directly
         (bypasses AuthProxy entirely — clientFd = -1, logToAof = false so replay
          doesn't re-append what it just replayed)
  6. server.start()
       → bind/listen on :6379, then loop: accept() → spawn detached std::thread → handleClient(fd)
```

Step 4/5 is a **precedence choice, not a merge**: snapshot and AOF are treated as alternatives.
See §7 for why, and what that costs.

---

## 4. Request Lifecycle

### 4.1 A normal command — `SET foo bar PX 1000`

```
client socket
  → TcpServer::handleClient() blocking read() into a 4096-byte buffer
  → RespParser::parse(raw) → tokens = ["SET", "foo", "bar", "PX", "1000"]
  → AuthProxy::getInstance().dispatch(_dispatcher, tokens, clientFd)
      if AuthProxy is enabled (a password was configured) and tokens[0] != "AUTH"
      and this clientFd is not in the authenticated set → short-circuit, return
      "-NOAUTH Authentication required.\r\n" without touching the dispatcher at all
  → (authorized) CommandDispatcher::dispatch(tokens, clientFd, logToAof=true)
      → factory lambda for "SET" builds SetCommand("foo", "bar", ttlMs=1000)
      → SetCommand::execute()
          → StoreEngine::set(): lock _mutex, insert/overwrite DataEntry(StringType("bar"), 1000),
            DataEntry's constructor converts the relative 1000ms into an absolute expiry
            (now + 1000), evictionPolicy->onSet("foo") updates LRU recency, unlock
          → returns "+OK\r\n"
      → since "SET" was registered with isWriteCommand=true, AofWriter::append(tokens) is
        called — writes "SET foo bar PX 1000\n" to appendonly.aof and flushes
  → TcpServer writes the "+OK\r\n" reply back to the client socket
```

Any exception thrown anywhere in that chain (bad key, wrong type, malformed command) is
caught by `handleClient`'s `catch (const std::exception&)` and turned into a generic
`-ERR <what()>\r\n` reply — commands don't need their own top-level error handling for that case.

### 4.2 Authentication — `AUTH`

`AUTH` is dispatched like any other command (`AuthCommand`, clientFd-aware, same shape as
`SubscribeCommand`) — `AuthProxy::dispatch()` explicitly lets `tokens[0] == "AUTH"` through
regardless of auth state, so a client can always attempt to authenticate. `AuthCommand::execute()`
calls `AuthProxy::authenticate(clientFd, password)`, which compares against the configured
password and, on success, inserts `clientFd` into `AuthProxy`'s `_authenticatedClients` set.
Every subsequent command on **that same TCP connection** now passes the gate. A different
connection — even from the same client process — is a different fd and starts unauthenticated.

`AuthProxy::removeClient(clientFd)` is called when `handleClient`'s read loop exits (client
disconnected), clearing that fd out of the authenticated set. This matters because the OS
recycles fd numbers: without this cleanup, a brand-new connection that happens to be assigned
a previously-authenticated fd would inherit that authentication for free.

### 4.3 Pub/Sub — two connections, one channel

```
Connection A: SUBSCRIBE chan1
  → SubscribeCommand::execute() → PubSubManager::subscribe("chan1", fdA)
  → reply: "*3\r\n$9\r\nsubscribe\r\n$5\r\nchan1\r\n:1\r\n"
  → A's handleClient thread now just sits blocked in read(), waiting for the next
    client-initiated message — it does NOT poll for incoming pub/sub messages itself

Connection B: PUBLISH chan1 hello
  → PublishCommand::execute() → PubSubManager::publish("chan1", "hello")
      → looks up _channels["chan1"] = [fdA, ...]
      → for each subscriber fd: write(fd, "*3\r\n$7\r\nmessage\r\n$5\r\nchan1\r\n$5\r\nhello\r\n", ...)
        — this write happens on B's thread, directly into a socket owned by A's connection
  → reply to B: ":1\r\n" (always — see §8, this doesn't reflect actual subscriber count)

Connection A's next read() call returns the bytes B's thread just wrote — the message
arrives as unsolicited bytes on a connection that didn't ask for anything at that moment.
```

This cross-thread `write()` into another thread's socket is safe (POSIX guarantees `write()`
itself is thread-safe/atomic for a given fd against concurrent unrelated writers), but there's
no delivery acknowledgment and no cleanup of a subscriber's fd from `_channels` when that
client disconnects — see §8.

### 4.4 Persistence — `SAVE` and restart

```
SAVE
  → SaveCommand::execute() → SnapshotWriter::save("dump.rdb")
      → StoreEngine::forEachEntry(callback): locks _mutex once, iterates the whole map,
        skips anything already isExpired(), and for each surviving entry writes:
          [u32 keyLen][key bytes][i64 absoluteExpiryMs][u8 typeTag][type-specific payload]
        String  → [u32 len][bytes]
        List    → [u32 count][u32 len][bytes] × count
        Hash    → [u32 count]([u32 keyLen][key][u32 valLen][val]) × count
        Set     → [u32 count][u32 len][bytes] × count
      → file starts with magic "MRDB" + a version byte
  → reply: "+OK\r\n" (not AOF-logged — SAVE isn't registered as a write command)

Next process restart
  → loadSnapshot() reads dump.rdb; for each entry, if its absolute expiry has already
    passed relative to *now*, it's skipped (never restored) — expiry is evaluated at
    load time, not at save time, so downtime between SAVE and restart counts against the TTL
  → if dump.rdb is missing entirely, falls back to full AOF replay instead (§3)
```

This binary format is why `SnapshotWriter` doesn't inherit the delimiter-corruption bugs
in `IDataType::serialize()` (§8) — it reads/writes each type's raw `getValue()` directly,
length-prefixed, instead of going through the lossy string format.

---

## 5. Concurrency Model

- **One OS thread per client connection**, spawned via `std::thread(...).detach()` in
  `TcpServer::start()`'s accept loop. No thread pool, no connection limit beyond the OS and
  `listen()`'s backlog of 10.
- Each thread runs its own blocking `read()` loop in `handleClient()` until the client
  disconnects or sends 0 bytes.
- Shared mutable state and what protects it:

  | Shared state | Guarded by |
  |---|---|
  | `StoreEngine::_store` | `StoreEngine::_mutex` (internal `lock_guard` on every public method) |
  | `PubSubManager::_channels` | `PubSubManager::_mutex` |
  | `AofWriter`'s file handle | `AofWriter::_mutex` |
  | `AuthProxy::_authenticatedClients` | `AuthProxy::_mutex` |

- **Load-bearing invariant, not enforced by the type system**: `IEvictionPolicy`
  implementations (`LRUPolicy`, `LFUPolicy`) have **no mutex of their own**. They're safe today
  only because `StoreEngine::get()`/`set()`/`evict()` always call `_evictionPolicy->onGet()` /
  `onSet()` / `evict()` from *inside* a scope already holding `StoreEngine::_mutex`. If a future
  change calls an eviction policy method from anywhere else, that call is unprotected.
- `AuthProxy::_password` is written once, before `TcpServer::start()` spawns any threads, and
  never again — reads of it from multiple threads afterward are safe by construction (no
  concurrent writer), not because of a lock.

---

## 6. Eviction — Wired but Not Triggered

`StoreEngine` holds a `unique_ptr<IEvictionPolicy>` and calls `onGet`/`onSet` on every access,
so both `LRUPolicy` (doubly-linked-list-style recency via `std::list` + iterator map) and
`LFUPolicy` (frequency buckets keyed by access count, `_minFreq` tracking the eviction
candidate bucket) maintain fully correct internal state at all times.

**Nothing calls `StoreEngine::evict()`.** There's no command that exposes it (no `DBSIZE`,
no memory-limit config, no periodic sweep), and no automatic trigger tied to store size or
process memory. The two eviction strategies are fully implemented and unit-correct, but
inert in the running server today — this is the most likely thing to surprise a future
contributor who assumes "LRU is configured" means "keys actually get evicted."

---

## 7. Persistence — Two Independent Layers, Deliberately Not Merged

| | AOF (`AofWriter`) | Snapshot (`SnapshotWriter`) |
|---|---|---|
| Format | Whitespace-joined plain-text tokens, one command per line | Binary, length-prefixed, typed |
| Written | On every successful write command (`SET`/`DEL`/`LPUSH`/`HSET`) | Only when `SAVE` is called |
| Restored by | Replaying every line through the dispatcher from scratch | Direct binary deserialization into the store |
| Startup precedence | Fallback — used only if `dump.rdb` doesn't exist | Preferred — tried first |

Real Redis layers these (an RDB base plus the AOF written *since* that snapshot), but that
requires tracking which AOF byte offset a snapshot corresponds to, so replay only applies
what's new. This project's AOF is append-only forever and never truncated or offset-tracked,
so replaying the full AOF on top of an already-restored snapshot would redo everything the
snapshot already contains. For idempotent commands (`SET`, `DEL`, `HSET` overwrite) that's
merely wasted work; for `LPUSH` it would silently duplicate list entries. Treating snapshot
and AOF as **alternatives** (§3) — not layers — sidesteps that correctness trap at the cost
of losing any AOF-only writes once a snapshot has been taken and restored from.

---

## 8. Known Limitations

Ordered roughly by how likely each is to surprise someone:

1. **`RespParser::parse` can silently corrupt values that start with `*` or `$`.**
   The parser splits the raw input into lines and simply discards any line whose first
   character is `*` or `$`, assuming that only ever means "this is a RESP array/bulk-string
   length header." A client sending `SET key *anything` or `SET key $5000` has that value's
   own line dropped as if it were protocol metadata — the token vector silently shrinks by
   one, downstream `tokens[N]` indexing shifts, and the command executes against the wrong
   argument (verified: `SET weird "*starts-with-star"` stores an **empty string**, not the
   intended value). This is a real protocol-desync bug reachable from any client input, not
   just a theoretical edge case — worth fixing before this server sees any input it doesn't
   fully control.

2. **`ListType`/`HashType`/`SetType` serialize to ad-hoc delimited strings.**
   `ListType` joins with `,`, `HashType` with `key:value|key:value`, `SetType` with `|` —
   each breaks the moment a stored value contains that delimiter. `SnapshotWriter`'s binary
   format doesn't have this problem (it reads/writes raw `getValue()`s directly), which is
   why snapshot save/restore was explicitly tested with values containing `,`/`:`/`|` and
   AOF/`serialize()` was not.

3. **`appendonly.aof` is plain whitespace-joined tokens, not RESP-encoded.**
   A key or value containing a space breaks both the on-disk format and replay (which
   `std::istringstream >> token`-splits on whitespace rather than reusing `RespParser`).

4. **Eviction is inert** — see §6. `LRUPolicy`/`LFUPolicy` track state correctly but
   `StoreEngine::evict()` is never invoked by any reachable code path.

5. **Expired keys are only reclaimed lazily, and only via `GET`.** `DataEntry::isExpired()`
   is checked in `StoreEngine::get()` (which erases the entry on a hit) and in
   `SnapshotWriter`'s save-time iteration (which skips but does not delete expired entries).
   A key that expires and is never `GET` again, and is never overwritten by `SET`/`DEL`,
   stays in the in-memory map indefinitely — there is no background sweep.

6. **`PubSubManager` never removes a disconnected client's fd from `_channels`.**
   If a subscriber disconnects without sending `UNSUBSCRIBE`, its fd lingers in the channel's
   subscriber list forever. A later `PUBLISH` will `write()` to that closed fd — harmless in
   that it won't crash the process, but it's a silent no-op that masks the fact the channel
   has fewer live subscribers than `_channels` suggests.

7. **`PublishCommand` always replies `:1\r\n`**, regardless of how many subscribers (zero,
   one, or many) actually received the message — it doesn't return `PubSubManager::publish()`'s
   actual delivery count because that method doesn't currently return one.

8. **Snapshot and AOF are alternatives, not layered** — see §7. A `SAVE` followed by more
   writes, followed by a restart, loses everything written after that `SAVE` (verified in the
   Day 25 end-to-end test — this is intentional current behavior, not a bug, but easy to
   assume otherwise).

9. **`DataTypeFactory` is unused in the actual command/persistence code paths.** Every command
   and `SnapshotWriter` construct `StringType`/`ListType`/`HashType`/`SetType` directly with
   real initial values, because `DataTypeFactory::create()` only knows how to build an *empty*
   instance of a named type. It's kept as the project's canonical Factory-pattern example (see
   §9) rather than removed, since demonstrating the pattern is part of this project's purpose —
   but don't go looking for it on the hot path.

---

## 9. Design Patterns — Where and Why

| Pattern | Class(es) | File | One-line why |
|---|---|---|---|
| Singleton | `StoreEngine`, `PubSubManager`, `AofWriter`, `AuthProxy` | `src/store/StoreEngine.*`, `src/pubsub/PubSubManager.*`, `src/persistence/AofWriter.*`, `src/auth/AuthProxy.*` | Exactly one store, one channel registry, one AOF file handle, one auth gate per process |
| Factory | `DataTypeFactory`; `CommandFactory` lambdas in `TcpServer`'s constructor | `src/store/types/DataTypeFactory.*`; `src/server/TcpServer.cpp` | Lambdas can close over per-request tokens (key/value/ttl) that a pre-built object couldn't carry |
| Strategy | `IEvictionPolicy` → `LRUPolicy` / `LFUPolicy` | `src/eviction/*` | `StoreEngine` calls `onGet`/`onSet`/`evict()` without knowing which policy is active; swappable at startup in `main.cpp` |
| Command | `ICommand` → `GetCommand`, `SetCommand`, `AuthCommand`, ... | `src/commands/*` | Every verb is an object with a single `execute()`; `CommandDispatcher` treats them uniformly |
| Observer | `PubSubManager` | `src/pubsub/PubSubManager.*` | Channel → subscriber-fd list; `publish()` notifies every registered observer by writing directly to its socket |
| Decorator | `DataEntry` wrapping `IDataType` | `src/store/DataEntry.*` | Adds TTL metadata without touching the `IDataType` contract that `StringType`/`ListType`/etc. implement |
| Proxy | `AuthProxy` in front of `CommandDispatcher` | `src/auth/AuthProxy.*` | Same call shape (`dispatch(tokens, clientFd)`) as the real subject; decides whether to forward or reject before the real dispatcher ever sees the call |

---

## 10. Extension Points (if picking this back up)

- **Fix the parser bug (§8.1) first** — it's the one that can silently corrupt otherwise
  correct client input, independent of any feature work.
- To make eviction real: decide a trigger (e.g. a max-key-count check in `StoreEngine::set()`
  that calls `evict()` when exceeded) — the policies themselves need no changes.
- To make persistence layered like real Redis: `SnapshotWriter::save()` would need to record
  the AOF's current byte offset (or line count) at save time, and `loadAof()` would need to
  seek past everything at-or-before that offset instead of replaying from the start.
- To fix `ListType`/`HashType`/`SetType` serialization: switch `serialize()` to a RESP-array
  encoding (`*N\r\n$len\r\nvalue\r\n...`) instead of ad-hoc delimiters — this only affects the
  `GET` reply and AOF text format; `SnapshotWriter` is already unaffected.
