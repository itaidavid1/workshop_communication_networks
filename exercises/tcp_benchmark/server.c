#define _POSIX_C_SOURCE 200809L

#include "common.h"

#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static int handle_client(int client_fd) {
    unsigned char *buffer = malloc(MAX_MESSAGE_SIZE);
    if (buffer == NULL) {
        perror("malloc");
        return -1;
    }

    for (;;) {
        struct benchmark_header header;
        if (recv_all(client_fd, &header, sizeof(header)) < 0) {
            free(buffer);
            return 0;
        }

        uint64_t message_size = ntohll(header.message_size);
        uint64_t message_count = ntohll(header.message_count);

        if (message_size < MIN_MESSAGE_SIZE || message_size > MAX_MESSAGE_SIZE || message_count == 0) {
            fprintf(stderr, "invalid benchmark request: size=%llu count=%llu\n",
                    (unsigned long long)message_size,
                    (unsigned long long)message_count);
            free(buffer);
            return -1;
        }

        for (uint64_t i = 0; i < message_count; ++i) {
            if (recv_all(client_fd, buffer, (size_t)message_size) < 0) {
                free(buffer);
                return -1;
            }
        }

        unsigned char ack = 1;
        if (send_all(client_fd, &ack, sizeof(ack)) < 0) {
            free(buffer);
            return -1;
        }
    }
}

int main(int argc, char **argv) {
    uint16_t port = DEFAULT_PORT;
    if (argc > 2) {
        print_usage_server(argv[0]);
        return EXIT_FAILURE;
    }
    if (argc == 2) {
        char *end = NULL;
        long parsed = strtol(argv[1], &end, 10);
        if (end == argv[1] || *end != '\0' || parsed <= 0 || parsed > 65535) {
            print_usage_server(argv[0]);
            return EXIT_FAILURE;
        }
        port = (uint16_t)parsed;
    }

    int listen_fd = create_listening_socket(port);
    if (listen_fd < 0) {
        return EXIT_FAILURE;
    }

    for (;;) {
        struct sockaddr_in client_address;
        socklen_t client_length = sizeof(client_address);
        int client_fd = accept(listen_fd, (struct sockaddr *)&client_address, &client_length);
        if (client_fd < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("accept");
            close(listen_fd);
            return EXIT_FAILURE;
        }

        (void)handle_client(client_fd);
        close(client_fd);
    }

    close(listen_fd);
    return EXIT_SUCCESS;
}