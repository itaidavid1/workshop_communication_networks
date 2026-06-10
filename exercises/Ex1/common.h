#ifndef TCP_BENCHMARK_COMMON_H
#define TCP_BENCHMARK_COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define DEFAULT_PORT 45678
#define MAX_MESSAGE_SIZE (1024 * 1024)
#define MIN_MESSAGE_SIZE 1
/* Warm-up rounds (3): the first few rounds are discarded because TCP needs
 * time to ramp up its congestion window (slow start) and the OS socket buffers
 * need to fill before the link is fully utilized. Without warm-up, the first
 * measured round would report artificially low throughput.
 * Measured rounds (5): taking the best of 5 rounds filters out transient
 * interference from other processes or network jitter, while keeping the total
 * benchmark runtime reasonable. */
#define WARMUP_ROUNDS 3
#define MEASURE_ROUNDS 5

struct benchmark_header {
    uint64_t message_size;
    uint64_t message_count;
};

/* Return current monotonic time in seconds with sub-second precision. */
double monotonic_seconds(void);

/* Clamp an unsigned 64-bit value between minimum and maximum. */
uint64_t clamp_u64(uint64_t value, uint64_t minimum, uint64_t maximum);

/* Choose a sensible message count for the given message size. */
uint64_t choose_message_count(size_t message_size);

/* Compute total transferred bytes from message size and count. */
uint64_t bytes_to_human_count(size_t message_size, uint64_t message_count);

/* 64-bit host/network byte order helpers. */
uint64_t htonll(uint64_t value);
uint64_t ntohll(uint64_t value);

/* Socket helpers: create a listening TCP socket, or connect to a host. */
int create_listening_socket(uint16_t port);
int connect_to_host(const char *host, uint16_t port);

/* Reliable I/O helpers: loop until the full buffer is sent/received. */
int send_all(int socket_fd, const void *buffer, size_t length);
int recv_all(int socket_fd, void *buffer, size_t length);

/* Print usage strings for server and client. */
void print_usage_server(const char *program_name);
void print_usage_client(const char *program_name);

#endif