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

The full rationale for these choices is documented in `common.h` next to the `WARMUP_ROUNDS` and `MEASURE_ROUNDS` defines.

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
| 1 B | 1.614 | MiB/s |
| 2 B | 3.342 | MiB/s |
| 4 B | 6.656 | MiB/s |
| 8 B | 13.272 | MiB/s |
| 16 B | 26.656 | MiB/s |
| 32 B | 52.833 | MiB/s |
| 64 B | 94.479 | MiB/s |
| 128 B | 111.651 | MiB/s |
| 256 B | 111.739 | MiB/s |
| 512 B | 111.938 | MiB/s |
| 1 KiB | 111.872 | MiB/s |
| 2 KiB | 111.895 | MiB/s |
| 4 KiB | 111.975 | MiB/s |
| 8 KiB | 111.921 | MiB/s |
| 16 KiB | 111.911 | MiB/s |
| 32 KiB | 111.890 | MiB/s |
| 64 KiB | 111.835 | MiB/s |
| 128 KiB | 111.747 | MiB/s |
| 256 KiB | 111.923 | MiB/s |
| 512 KiB | 111.934 | MiB/s |
| 1 MiB | 112.040 | MiB/s |