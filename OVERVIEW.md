# mini-Redis — Overview

> 3-minute read. If you need build steps or the full command list, see `README.md`.
> If you need to understand *how* something works internally, see `HLD.md`.

---

## What This Is

A Redis clone written from scratch in C++17 — a single-process TCP server that speaks a
subset of the RESP protocol on port 6379, backed by an in-memory hash map with optional
TTL, pub/sub, two persistence mechanisms, and an auth gate. Built as a structured, one
-design-pattern-at-a-time learning project, not as production infrastructure.

If you're picking this up cold (new to the repo, or the original author returning after
months away): this file plus `HLD.md` should be enough to get oriented without reading
every source file first.

---

## What It Actually Does

- **Data types**: String, List, Hash, Set — each stored in the same `unordered_map<string, DataEntry>`
- **TTL**: optional per-key expiry (`SET key value PX <ms>`), checked lazily on access
- **Eviction**: LRU and LFU policies are fully implemented and track access patterns correctly,
  but nothing currently triggers an eviction — see `HLD.md` §6 before assuming otherwise
- **Pub/Sub**: `SUBSCRIBE`/`PUBLISH`/`UNSUBSCRIBE`, channel broadcast to multiple subscribers
- **Persistence, two independent mechanisms** (not layered — see `HLD.md` §7):
  - AOF: every write command appended to `appendonly.aof`, replayed on startup
  - Snapshot: `SAVE` binary-dumps the whole store to `dump.rdb`; preferred over AOF replay
    on startup if present
- **Auth**: optional password passed as a CLI arg (`./mini-redis <password>`); once set,
  every command except `AUTH` is blocked per-connection until that connection authenticates

Known rough edges (parser bug, delimiter-breaking serialization, inert eviction, etc.) are
tracked in `HLD.md` §8 and in `README.md`'s Known Limitations section — read those before
assuming a given piece of behavior is intentional.

---

## Request Flow, One Line Each

```
TCP socket → RespParser (bytes → tokens) → AuthProxy (auth gate) →
CommandDispatcher (tokens → Command object) → Command.execute() →
StoreEngine / PubSubManager / AuthProxy / SnapshotWriter → RESP reply string → socket
```

One `std::thread` per connected client; `StoreEngine`, `PubSubManager`, `AofWriter`, and
`AuthProxy` are each a mutex-protected singleton. Full walkthrough with a concrete `SET` and
`AUTH` example: `HLD.md` §4.

---

## Design Patterns, and Where to Find Them

| Pattern | Where | What it buys you |
|---|---|---|
| **Singleton** | `StoreEngine`, `PubSubManager`, `AofWriter`, `AuthProxy` | One shared instance per process, accessed via `getInstance()` |
| **Factory** | `DataTypeFactory`; the command-registration lambdas in `TcpServer`'s constructor | Building an object from runtime data (a parsed command's tokens) instead of a fixed constructor call |
| **Strategy** | `IEvictionPolicy` → `LRUPolicy` / `LFUPolicy` | Swap the eviction algorithm at startup without `StoreEngine` knowing which one is active |
| **Command** | `ICommand` → one class per verb (`GetCommand`, `SetCommand`, `AuthCommand`, ...) | Every request becomes an object with a uniform `execute()`; the dispatcher doesn't special-case any verb |
| **Observer** | `PubSubManager` | Channels notify every subscribed connection by writing straight to its socket |
| **Decorator** | `DataEntry` wrapping `IDataType` | TTL metadata bolted onto a value without changing what a "value" is |
| **Proxy** | `AuthProxy` in front of `CommandDispatcher` | Auth-gating lives in one place, outside every command, and is a no-op when no password is configured |

Each pattern's full rationale (the "why this pattern and not something simpler") is in
`HLD.md` §9 and in `README.md`'s "Key Design Decisions" section.

---

## Where Things Live

```
src/
├── main.cpp          — startup wiring: eviction policy, optional auth password, load
│                        persistence, start server
├── server/           — TcpServer: socket accept loop, thread-per-client, command registration
├── protocol/          — RespParser: raw bytes → tokens
├── auth/              — AuthProxy: the Proxy gating access to CommandDispatcher
├── commands/          — CommandDispatcher + one *Command class per verb
├── store/             — StoreEngine (the actual map), DataEntry (TTL decorator),
│                        types/ (String, List, Hash, Set + DataTypeFactory)
├── eviction/           — IEvictionPolicy, LRUPolicy, LFUPolicy
├── pubsub/             — PubSubManager
└── persistence/        — AofWriter (text log), SnapshotWriter (binary dump.rdb)
```

---

## Building and Running

```bash
make init && make build   # first time
make run                  # ./build/mini-redis, no auth
./build/mini-redis <password>   # same binary, auth enabled
```

Full command list and `redis-cli` usage examples: `README.md`.

---

## Where to Go Next

- **Just want to use it / see the command list** → `README.md`
- **Need to understand a mechanism in depth (concurrency, persistence precedence, why a
  bug exists)** → `HLD.md`
- **Picking a first thing to fix** → `HLD.md` §10 "Extension Points", ordered by
  what's most worth doing first
