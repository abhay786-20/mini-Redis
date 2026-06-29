# Mini Redis — C++

A minimal in-memory key-value store built in C++, inspired by Redis.
Built as a system design + design patterns learning project.

## Features (Progressive)
- Core data types: String, List, Hash, Set
- TTL and eviction policies: LRU, LFU
- Pub/Sub
- TCP Server with RESP protocol
- Persistence: AOF + Snapshot

## Design Patterns Used
- Singleton — StoreEngine
- Factory — DataType creation
- Strategy — Eviction policies
- Command — Redis commands
- Observer — Pub/Sub
- Decorator — TTL wrapping
- Proxy — Auth layer

## Build & Run

```bash
mkdir build && cd build
cmake ..
cmake --build .
./mini-redis
```

## Project Structure