# ErktDB

A key-value store written from scratch in C, as a learning project. A master node routes client requests to a ring of worker nodes over a custom binary protocol on TCP. No frameworks, no HTTP, no JSON: only Linux system calls, sockets and `epoll`.

## What it does

- **Non-blocking I/O:** the master and every worker run a single-threaded `epoll` loop. Each connection has a small state machine (waiting for header → waiting for payload) so partial reads are handled.
- **Custom binary protocol:** fixed-size headers followed by the raw key and value bytes. Header fields are serialized by hand in big-endian (network byte order) by the `serialize_*` / `deserialize_*` functions in `src/common/net.c`.
- **Routing:** the master hashes the key (djb2) modulo the number of workers to pick the target worker.
- **Storage:** each worker keeps an in-memory hash map with separate chaining (4096 buckets), plus a second map that holds the backup copy of its predecessor's data.
- **Replication:** after a successful `POST` or `DEL`, a worker forwards the same request to the next worker in the ring, which stores it in its backup map. Replication is asynchronous: the client's response does not wait for the backup.
- **Persistence:** on `CTRL+C` a worker writes its primary map to `<port>.db` and its backup map to `<predecessor-port>.db.bc`, and loads both on startup.

## Protocol

| Header | Size | Fields |
|---|---|---|
| Public (client → master) | 12 bytes | magic (2), op (1), padding (1), key length (4), value length (4) |
| Private (master ↔ worker, worker → backup) | 25 bytes | public header (12), client fd (4), request id (8), flag (1: 0 = primary, 1 = backup) |
| Response (worker → master) | 20 bytes | request id (8), client fd (4), message length (4), status (4) |

Operations: `GET`, `POST`, `DEL`, plus an internal `SYNC_START`.

## Known limitations

- Fixed configuration: three workers on `127.0.0.1:8081–8083` and the master on port `19119` (`include/common/net.h`, `src/common/net.c`).
- The master connects to the workers only at startup; there is no heartbeat and no reconnection.
- Worker failure handling is incomplete and untested; do not rely on it.
- Asynchronous replication means a write acknowledged by the primary can be lost if the primary dies before the backup receives it.
- The `magic` field is not validated yet.
- The test client sends a single hard-coded `GET` request.

## Build and run

Requires Linux, `gcc` and `make`.

```bash
make
```

Start the three workers in separate terminals, within a few seconds of each other (each worker retries connecting to its successor three times, two seconds apart):

```bash
./worker_node 0 8081
./worker_node 1 8082
./worker_node 2 8083
```

Then start the master and the test client:

```bash
./master_node
./client_app
```
