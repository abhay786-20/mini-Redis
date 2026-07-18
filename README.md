# Mini Redis — C++

A minimal in-memory key-value store built from scratch in C++, inspired by Redis.
Built as a structured learning project covering systems programming, TCP networking,
and software design patterns — one 30-minute session per day.

---

## Why This Project

Most Redis tutorials show you how to *use* Redis. This project shows you how to *build* one.

By the end you'll have:
- A working TCP server compatible with `redis-cli`
- Deep understanding of 7 design patterns applied in real code
- Hands-on systems programming experience (sockets, threads, file I/O)
- A project you can fully explain and defend in any engineering interview

> **New here, or coming back after a while?** Start with [`OVERVIEW.md`](OVERVIEW.md) for a
> quick tour, then [`HLD.md`](HLD.md) for the architecture and request-lifecycle deep dive.

---

## Features

### Core Data Types
- **String** — simple key-value storage
- **List** — ordered collection with push/pop operations
- **Hash** — field-value map stored under one key
- **Set** — collection of unique values

### Commands Supported
| Command | Description |
|---|---|
| `SET key value` | Store a string value |
| `SET key value PX ms` | Store with TTL in milliseconds |
| `GET key` | Retrieve a value |
| `DEL key` | Delete a key |
| `LPUSH key value` | Push to a list |
| `HSET key field value` | Set a hash field |
| `SUBSCRIBE channel` | Subscribe to a Pub/Sub channel |
| `PUBLISH channel message` | Publish to a channel |
| `UNSUBSCRIBE channel` | Unsubscribe from a channel |
| `SAVE` | Snapshot the entire store to `dump.rdb` |
| `AUTH password` | Authenticate the current connection |

### Eviction Policies (Strategy Pattern)
- **LRU** — Least Recently Used
- **LFU** — Least Frequently Used
- **TTL** — Time To Live with lazy expiry

### Networking
- Raw POSIX TCP socket server on port 6379
- RESP (Redis Serialization Protocol) parser
- Multi-client support via `std::thread`
- Compatible with `redis-cli`

### Pub/Sub (Observer Pattern)
- Channel-based message broadcasting
- Multiple subscribers per channel
- Real-time message delivery to connected clients

### Persistence
- AOF (Append Only File) — logs every write command to `appendonly.aof`
- AOF Replay — reconstruct store state on startup by replaying the log
- Snapshot (RDB-style) — `SAVE` binary-serializes the whole store to `dump.rdb`;
  on startup the snapshot is preferred and AOF replay is the fallback if no snapshot exists

### Auth (Proxy Pattern)
- Optional password, passed as a CLI argument at startup: `./mini-redis <password>`
- `AuthProxy` gates every command except `AUTH` behind authentication, per connection
- No password provided → auth is disabled and the server behaves exactly as before

---

## Design Patterns Used

| Pattern | Where | Purpose |
|---|---|---|
| **Factory** | `DataTypeFactory` | Creates String/List/Hash/Set objects |
| **Singleton** | `StoreEngine`, `PubSubManager` | Single global instance |
| **Strategy** | `IEvictionPolicy` | Swap LRU/LFU at runtime |
| **Command** | `ICommand`, all command classes | Encapsulate requests as objects |
| **Observer** | `PubSubManager` | Notify subscribers on publish |
| **Decorator** | `DataEntry` | Wrap IDataType with TTL metadata |
| **Proxy** | `AuthProxy` | Gate access to `CommandDispatcher` until authenticated |

---

## Architecture

```
┌─────────────────────────────────────────────────────┐
│                   TCP Server Layer                   │
│         (accepts connections, spawns threads)        │
└──────────────────────┬──────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────┐
│              RESP Protocol Parser                    │
│         (deserialize raw bytes → tokens)             │
└──────────────────────┬──────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────┐
│                  Auth Proxy                          │
│  (gates every command except AUTH until authed;      │
│   no-op pass-through if no password configured)       │
└──────────────────────┬──────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────┐
│              Command Dispatcher                      │
│    (factory lambdas → builds command objects)        │
└───┬──────────────┬──────────────┬───────────────────┘
    │              │              │
┌───▼───┐    ┌────▼────┐   ┌─────▼──────┐
│ Store │    │ PubSub  │   │ Persistence│
│Engine │    │ Manager │   │  Manager   │
└───┬───┘    └─────────┘   └────────────┘
    │
┌───▼────────────────────────────────┐
│         Data Structure Layer       │
│  String | List | Hash | Set        │
└───┬────────────────────────────────┘
    │
┌───▼────────────────────────────────┐
│         Eviction Manager           │
│      LRU | LFU | TTL               │
└────────────────────────────────────┘
```

---

## Project Structure

```
mini-redis/
├── CMakeLists.txt
├── Makefile
├── src/
│   ├── main.cpp
│   ├── server/
│   │   └── TcpServer.hpp / .cpp        # TCP server, thread-per-client
│   ├── protocol/
│   │   └── RespParser.hpp / .cpp       # RESP protocol parser
│   ├── commands/
│   │   ├── ICommand.hpp                # Command interface
│   │   ├── CommandDispatcher.hpp/.cpp  # Routes commands via factory lambdas
│   │   ├── GetCommand.hpp / .cpp
│   │   ├── SetCommand.hpp / .cpp       # Supports PX flag for TTL
│   │   ├── DelCommand.hpp / .cpp
│   │   ├── LPushCommand.hpp / .cpp
│   │   ├── HSetCommand.hpp / .cpp
│   │   ├── SubscribeCommand.hpp / .cpp
│   │   ├── PublishCommand.hpp / .cpp
│   │   ├── UnsubscribeCommand.hpp / .cpp
│   │   ├── SaveCommand.hpp / .cpp
│   │   └── AuthCommand.hpp / .cpp
│   ├── store/
│   │   ├── StoreEngine.hpp / .cpp      # Singleton store
│   │   ├── DataEntry.hpp / .cpp        # Decorator — wraps IDataType with TTL
│   │   └── types/
│   │       ├── IDataType.hpp           # Abstract base interface
│   │       ├── StringType.hpp / .cpp
│   │       ├── ListType.hpp / .cpp
│   │       ├── HashType.hpp / .cpp
│   │       ├── SetType.hpp / .cpp
│   │       └── DataTypeFactory.hpp/.cpp
│   ├── eviction/
│   │   ├── IEvictionPolicy.hpp         # Strategy interface
│   │   ├── LRUPolicy.hpp / .cpp        # Doubly linked list + hashmap
│   │   └── LFUPolicy.hpp / .cpp        # Frequency buckets
│   ├── pubsub/
│   │   └── PubSubManager.hpp / .cpp    # Observer pattern
│   ├── persistence/
│   │   ├── AofWriter.hpp / .cpp        # Appends write commands to appendonly.aof
│   │   └── SnapshotWriter.hpp / .cpp   # Binary save()/load() of the whole store to dump.rdb
│   └── auth/
│       └── AuthProxy.hpp / .cpp        # Proxy pattern — gates CommandDispatcher access
└── tests/
```

---

## Build & Run

### First time setup
```bash
mkdir build && cd build && cmake .. && cd ..
```

### Commands
```bash
make run    # build and run server
make watch  # auto-rebuild on file save (requires entr)
make clean  # delete build folder
make init   # first time build setup
```

### Connect with redis-cli
```bash
redis-cli -p 6379

# Basic commands
SET name Abhay
GET name
DEL name

# TTL
SET session abc123 PX 5000
GET session   # works within 5 seconds
GET session   # ERR Key expired after 5 seconds

# Pub/Sub (requires two terminals)
SUBSCRIBE news        # terminal 1
PUBLISH news "hello"  # terminal 2

# Snapshot persistence
SAVE   # writes the whole store to dump.rdb; restored automatically on next startup
```

### Running with auth enabled
```bash
./build/mini-redis s3cret   # password is an optional CLI arg; omit it to disable auth

redis-cli -p 6379
AUTH s3cret
SET name Abhay   # NOAUTH error if you skip the AUTH step
```

---

## 25-Day Build Plan

### Week 1 — Foundation
| Day | What We Built |
|---|---|
| 1 | Project setup, CMake, `IDataType` interface |
| 2 | `StringType` — first concrete data type |
| 3 | `ListType`, `DataTypeFactory` (Factory pattern) |
| 4 | `HashType`, `SetType` |
| 5 | `StoreEngine` Singleton — core in-memory store |

### Week 2 — Commands + Protocol + Networking
| Day | What We Built |
|---|---|
| 6 | `ICommand` interface, `CommandDispatcher` with factory lambdas |
| 7 | `DelCommand`, `LPushCommand`, `HSetCommand` |
| 8 | RESP Protocol Parser |
| 9 | Parser wired to Dispatcher — full command pipeline |
| 10 | TCP Server — `redis-cli` talks to our server |

### Week 3 — Eviction + TTL
| Day | What We Built |
|---|---|
| 11 | `IEvictionPolicy` interface, `LRUPolicy` (Strategy pattern) |
| 12 | `DataEntry` Decorator — TTL support, lazy expiry |
| 13 | TCP server restored, TTL wired into `SetCommand` via PX flag |
| 14 | `LFUPolicy` — frequency bucket eviction |
| 15 | `PubSubManager` Singleton (Observer pattern structure) |

### Week 4 — Pub/Sub + Multi-Client
| Day | What We Built / Plan |
|---|---|
| 16 | `SUBSCRIBE`, `PUBLISH`, `UNSUBSCRIBE` commands wired |
| 17 | Multi-client threading — `std::thread` per client |
| 18 | Real `clientFd` passed to Pub/Sub commands |
| 19 | Thread safety — mutex on StoreEngine |
| 20 | End-to-end Pub/Sub test with two redis-cli clients |

### Week 5 — Persistence + Polish
| Day | What We Built |
|---|---|
| 21 | AOF Writer — append every write command to log file |
| 22 | AOF Replay — reconstruct store on startup from log |
| 23 | Snapshot — binary serialization of entire store |
| 24 | Auth Proxy layer — `AUTH` command, password gate |
| 25 | End-to-end test, cleanup, final README |

---

## Key Design Decisions

**Why factory lambdas in CommandDispatcher instead of pre-built objects?**
Pre-built objects can't carry runtime arguments (key, value from parsed tokens).
Lambdas are called fresh on every command with the actual token arguments.

**Why `DataEntry` wraps `IDataType` instead of adding TTL to `IDataType`?**
Single Responsibility. `IDataType` defines the value contract.
`DataEntry` adds metadata (TTL) without polluting the type interface.
This is the Decorator pattern — extend behavior without inheritance.

**Why `getRaw()` returns `DataEntry*` instead of `IDataType*`?**
`LPushCommand` and `HSetCommand` need to mutate the internal data (push to list,
set hash field). They need the actual object, not a serialized string.
`getRaw()` gives them a mutable pointer to the live object in the store.

**Why thread-per-client instead of epoll/select?**
Simpler to reason about for learning purposes. Each client gets its own stack,
its own blocking `read()` loop, no callback hell. Production Redis uses an
event loop (ae.c) but that's a separate learning track.

**Why does `AuthProxy` wrap `CommandDispatcher` instead of checking auth inside each command?**
That's the Proxy pattern: a stand-in with the same call shape as the real subject
(`dispatch(tokens, clientFd)`), sitting in front of it to control access. Commands stay
unaware that auth exists at all — `TcpServer` calls `AuthProxy::dispatch()` instead of
`CommandDispatcher::dispatch()` directly, and the proxy decides whether to forward the call.

**Why does snapshot restore take precedence over AOF replay on startup, instead of combining both?**
Real Redis layers them (RDB base + AOF-since-snapshot), but that requires tracking which AOF
offset the snapshot covers so it isn't double-applied. This project's AOF is never truncated,
so replaying it after a snapshot restore would redo everything already in the snapshot —
harmless for idempotent commands like `SET`/`DEL`, but wrong for accumulating ones like `LPUSH`.
Treating snapshot and AOF as alternatives (snapshot if present, else full AOF replay) sidesteps
that correctness trap. See `HLD.md` for the full write-up.

---

## Known Limitations

See [`HLD.md`](HLD.md#known-limitations) for the full list and reasoning. In brief:
- `ListType`/`HashType`/`SetType` serialize to delimited strings, which corrupts on values
  containing that delimiter (`,`, `:`, `|`). Only `SnapshotWriter`'s binary format avoids this.
- `appendonly.aof` is plain whitespace-joined tokens — breaks on spaces inside a key/value.
- `StoreEngine::evict()` is never invoked automatically; eviction policies track access but
  nothing enforces a memory cap yet.
- Snapshot and AOF are alternatives, not layered (see design decision above) — a snapshot
  taken mid-session drops any AOF-only history once restored.

---

## Tech Stack

- **Language:** C++17
- **Build:** CMake + Makefile
- **Networking:** Raw POSIX sockets
- **Threading:** `std::thread` (C++11)
- **Dev tooling:** `entr` for file watching (like Air in Go)
- **Testing:** `redis-cli` as live test client

---

## Interview Talking Points

- "I built a Redis clone to deeply understand how in-memory stores work at the systems level"
- "The command layer uses the Command pattern with factory lambdas so adding new commands never requires touching the dispatcher"
- "Eviction policies are swappable at runtime via the Strategy pattern — StoreEngine has no knowledge of which policy is active"
- "TTL is implemented as lazy expiry using a Decorator — each store entry carries an optional expiry timestamp checked on access"
- "Pub/Sub uses the Observer pattern — PubSubManager holds channel → subscriber file descriptor mappings and writes RESP directly to client sockets"
- "Auth is a Proxy sitting in front of the command dispatcher — commands have zero awareness that authentication exists, and it's a no-op pass-through when no password is configured"
- "Persistence has two independent layers: an AOF that's replayed from scratch, and a binary RDB-style snapshot that takes precedence when present — I can explain the tradeoff of not layering them like real Redis does"
