#define _POSIX_C_SOURCE 200809L

#include "common.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Perform one benchmark round: send header, all messages, wait for ack, and measure time. */
static int run_single_round(int socket_fd,
                            const unsigned char *payload,
                            size_t message_size,
                            uint64_t message_count,
                            double *elapsed_seconds) {
    struct benchmark_header header;
    header.message_size = htonll((uint64_t)message_size);
    header.message_count = htonll(message_count);

    if (send_all(socket_fd, &header, sizeof(header)) < 0) {
        return -1;
    }

    double start = monotonic_seconds();
    for (uint64_t i = 0; i < message_count; ++i) {
        if (send_all(socket_fd, payload, message_size) < 0) {
            return -1;
        }
    }

    unsigned char ack = 0;
    if (recv_all(socket_fd, &ack, sizeof(ack)) < 0) {
        return -1;
    }

    double finish = monotonic_seconds();
    *elapsed_seconds = finish - start;
    return 0;
}

/* Convert bytes/sec to MiB/s. */
static double bytes_per_second_to_mib_per_second(uint64_t total_bytes, double elapsed_seconds) {
    if (elapsed_seconds <= 0.0) {
        return 0.0;
    }
    return (double)total_bytes / elapsed_seconds / (1024.0 * 1024.0);
}

int main(int argc, char **argv) {
    /* Client: connect to server, iterate message sizes, perform warm-up and measured rounds, print best throughput per size. */
    if (argc < 2 || argc > 3) {
        print_usage_client(argv[0]);
        return EXIT_FAILURE;
    }

    const char *host = argv[1];
    uint16_t port = DEFAULT_PORT;
    if (argc == 3) {
        char *end = NULL;
        long parsed = strtol(argv[2], &end, 10);
        if (end == argv[2] || *end != '\0' || parsed <= 0 || parsed > 65535) {
            print_usage_client(argv[0]);
            return EXIT_FAILURE;
        }
        port = (uint16_t)parsed;
    }

    int socket_fd = connect_to_host(host, port);
    if (socket_fd < 0) {
        return EXIT_FAILURE;
    }

    unsigned char *payload = malloc(MAX_MESSAGE_SIZE);
    if (payload == NULL) {
        perror("malloc");
        close(socket_fd);
        return EXIT_FAILURE;
    }
    memset(payload, 'A', MAX_MESSAGE_SIZE);

    for (size_t message_size = MIN_MESSAGE_SIZE; message_size <= MAX_MESSAGE_SIZE; message_size <<= 1) {
        uint64_t message_count = choose_message_count(message_size);
        double best_mib_per_second = 0.0;

        for (int round = 0; round < WARMUP_ROUNDS + MEASURE_ROUNDS; ++round) {
            double elapsed_seconds = 0.0;
            if (run_single_round(socket_fd, payload, message_size, message_count, &elapsed_seconds) < 0) {
                perror("benchmark round");
                free(payload);
                close(socket_fd);
                return EXIT_FAILURE;
            }

            if (round >= WARMUP_ROUNDS) {
                uint64_t total_bytes = bytes_to_human_count(message_size, message_count);
                double mib_per_second = bytes_per_second_to_mib_per_second(total_bytes, elapsed_seconds);
                if (mib_per_second > best_mib_per_second) {
                    best_mib_per_second = mib_per_second;
                }
            }
        }

        printf("%zu\t%.3f\tMiB/s\n", message_size, best_mib_per_second);
        fflush(stdout);
    }

    free(payload);
    close(socket_fd);
    return EXIT_SUCCESS;
}