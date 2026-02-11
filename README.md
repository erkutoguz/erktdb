# Distri-DB: Distributed Key-Value Store in C

Hey,

**Distri-DB** is a distributed key-value store that I built from scratch using **C**.
I didn't use any high-level frameworks or libraries (like HTTP or JSON) because I wanted to understand how distributed systems actually work under the hood. I implemented everything using **Linux System Calls**, **TCP Sockets**, and **Epoll**.

It supports **Replication**, **Fault Tolerance**, and **Automatic Recovery**.

### Note:
This project is designed for learning purposes. It works correctly for the scenarios I tested, I know it's not much but I learned a lot.

---

## 🔧 Custom Binary Protocol
I didn't use slow text-based protocols like JSON or HTTP. Instead, I designed a **Custom Binary Protocol** to make communication as fast as possible.
Everything is sent as raw bytes using `packed` structs. This means there is **zero parsing overhead**.
Here is the exact structure I used in C:

```c
// 1. Public Header (12 Bytes)
// Every packet starts with this.
struct pubhdr {
    i16 magic;       // 2 bytes (To verify protocol integrity)
    enum OP op : 8;  // 1 byte  (Operation: GET, POST, DEL, SYNC)
    i8 pad;          // 1 byte  (Padding for alignment)
    i32 keylen;      // 4 bytes (Length of the Key)
    i32 vallen;      // 4 bytes (Length of the Value)
};

// 2. Private Header (25 Bytes)
// Used internally between Master and Workers.
// I used __attribute__((packed)) to prevent memory padding!
struct prihdr {
    struct pubhdr pub;  // 12 bytes (Includes public header)
    i32 client_fd;      // 4 bytes  (To route response back to client)
    i64 uuid;           // 8 bytes  (Unique Request ID)
    i8 flag;            // 1 byte   (0: Primary Write, 1: Replica Write)
} __attribute__((packed));

// 3. Response Header (20 Bytes)
// Sent back to the client.
struct rethdr {
    i64 uuid;       // 8 bytes
    sfd client_fd;  // 4 bytes
    i32 msglen;     // 4 bytes (Length of the response message)
    STAT status;    // 4 bytes (OK, FAIL, SERV_ERR)
} __attribute__((packed));
```

## Architecture

I used a **Master-Worker** architecture with a **Ring Topology**.

* **Master Node:** It manages the connections and redirects clients to the correct worker. It also detects if a worker is offline.
* **Worker Nodes:** They store the data in memory. Each worker is a **Primary** for its own data and a **Backup** for the previous worker in the ring.
* **Protocol:** I wrote my own custom **Binary Protocol**. It is much faster than text-based protocols.

## Features

* **Custom Binary Protocol:** No overhead, just raw bytes.
* **Non-Blocking I/O:** Uses `epoll` to handle multiple connections at the same time.
* **Replication:** When you write data to a worker, it automatically sends a backup copy to the next worker.
* **Failover:** If a worker crashes, the system doesn't stop. The Master redirects traffic to the backup.
* **Recovery (Self-Healing):** When a crashed worker comes back online, it automatically asks for its data from the backup and syncs itself.
* **Persistence:** It saves data to `.db` files when you stop it with `CTRL+C`.

---

##  Usage

### 1. Build the Project
You need `gcc` and `make`.

```bash
make
```

### 2. Run the Workers (Start these first)
You need to start workers before the master.
ID must start from 0 and go sequentially (0, 1, 2...).
PORT is where the worker listens.

# Open separate terminals for each worker:
```bash
./worker 0 8081
./worker 1 8082
./worker 2 8083
```

### 3. Run the Master Node
The master will automatically find the workers (configured in common.h).

```bash
./master_node
```
### 4. Run the Client
The client connects to the master and lets you send commands.

```bash
./client
```
