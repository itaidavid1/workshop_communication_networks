#ifndef TCP_BENCHMARK_COMMON_H
#define TCP_BENCHMARK_COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define DEFAULT_PORT 45678
#define MAX_MESSAGE_SIZE (1024 * 1024)
#define MIN_MESSAGE_SIZE 1
#define WARMUP_ROUNDS 3
#define MEASURE_ROUNDS 5

struct benchmark_header {
    uint64_t message_size;
    uint64_t message_count;
};

double monotonic_seconds(void);
uint64_t clamp_u64(uint64_t value, uint64_t minimum, uint64_t maximum);
uint64_t choose_message_count(size_t message_size);
uint64_t bytes_to_human_count(size_t message_size, uint64_t message_count);

uint64_t htonll(uint64_t value);
uint64_t ntohll(uint64_t value);

int create_listening_socket(uint16_t port);
int connect_to_host(const char *host, uint16_t port);

int send_all(int socket_fd, const void *buffer, size_t length);
int recv_all(int socket_fd, void *buffer, size_t length);

void print_usage_server(const char *program_name);
void print_usage_client(const char *program_name);

#endif