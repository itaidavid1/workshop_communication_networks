# Exercise 1: TCP Throughput Benchmark

## Implementation summary

This submission implements a two-process TCP benchmark in C:

- `server` accepts one client at a time and receives fixed-size payloads.
- `client` connects to the server, runs message sizes from 1 byte to 1 MiB, and prints one throughput result per size.
- The code uses a small shared protocol header to tell the server the payload size and message count for each benchmark round.

The code is optimized for low overhead:

- one persistent TCP connection per client run
- a single reusable payload buffer
- `send_all` and `recv_all` loops to avoid partial-transfer errors
- `-O3` build flags in the Makefile

## Measurement method

For each message size, the client runs several rounds over the same connection:

- 3 warm-up rounds to stabilize TCP state, buffering, and scheduler effects
- 5 measured rounds

The client reports the best measured throughput among the 5 measured rounds. That matches the goal of estimating the highest sustainable transmission rate for each message size.

The benchmark count is chosen so that each message size transfers a practical amount of data while keeping runtime reasonable. Small messages are capped so they do not run for too long, and large messages still transfer enough data to obtain a stable result.

## Build and run

Build:

```bash
make
```

Run server:

```bash
./server
```

Run client:

```bash
./client <server-host>
```

Optional port override:

```bash
./server 45678
./client <server-host> 45678
```

## Results

Fill this section after running the benchmark on the course hardware.

| Message size | Throughput | Unit |
| --- | --- | --- |
| 1 B | TBD | MiB/s |
| 2 B | TBD | MiB/s |
| 4 B | TBD | MiB/s |
| 8 B | TBD | MiB/s |
| 16 B | TBD | MiB/s |
| 32 B | TBD | MiB/s |
| 64 B | TBD | MiB/s |
| 128 B | TBD | MiB/s |
| 256 B | TBD | MiB/s |
| 512 B | TBD | MiB/s |
| 1 KiB | TBD | MiB/s |
| 2 KiB | TBD | MiB/s |
| 4 KiB | TBD | MiB/s |
| 8 KiB | TBD | MiB/s |
| 16 KiB | TBD | MiB/s |
| 32 KiB | TBD | MiB/s |
| 64 KiB | TBD | MiB/s |
| 128 KiB | TBD | MiB/s |
| 256 KiB | TBD | MiB/s |
| 512 KiB | TBD | MiB/s |
| 1 MiB | TBD | MiB/s |