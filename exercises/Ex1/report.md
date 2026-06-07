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
| 1 B | 1.428 | MiB/s |
| 2 B | 2.818 | MiB/s |
| 4 B | 5.624 | MiB/s |
| 8 B | 11.004 | MiB/s |
| 16 B | 21.980 | MiB/s |
| 32 B | 34.056 | MiB/s |
| 64 B | 35.568 | MiB/s |
| 128 B | 35.757 | MiB/s |
| 256 B | 36.024 | MiB/s |
| 512 B | 35.520 | MiB/s |
| 1 KiB | 35.814 | MiB/s |
| 2 KiB | 35.648 | MiB/s |
| 4 KiB | 36.432 | MiB/s |
| 8 KiB | 36.239 | MiB/s |
| 16 KiB | 36.371 | MiB/s |
| 32 KiB | 34.958 | MiB/s |
| 64 KiB | 37.179 | MiB/s |
| 128 KiB | 37.558 | MiB/s |
| 256 KiB | 37.507 | MiB/s |
| 512 KiB | 37.712 | MiB/s |
| 1 MiB | 48.206 | MiB/s |