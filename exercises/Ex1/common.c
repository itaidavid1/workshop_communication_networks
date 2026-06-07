#define _POSIX_C_SOURCE 200809L

#include "common.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

static uint64_t swap_u64(uint64_t value) {
    return ((value & 0x00000000000000ffULL) << 56) |
           ((value & 0x000000000000ff00ULL) << 40) |
           ((value & 0x0000000000ff0000ULL) << 24) |
           ((value & 0x00000000ff000000ULL) << 8) |
           ((value & 0x000000ff00000000ULL) >> 8) |
           ((value & 0x0000ff0000000000ULL) >> 24) |
           ((value & 0x00ff000000000000ULL) >> 40) |
           ((value & 0xff00000000000000ULL) >> 56);
}

/* Return current monotonic time in seconds (high resolution). */
double monotonic_seconds(void) {
    struct timespec ts;
#ifdef CLOCK_MONOTONIC_RAW
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
#else
    clock_gettime(CLOCK_MONOTONIC, &ts);
#endif
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
}

/* Clamp an unsigned 64-bit value to [minimum, maximum]. */
uint64_t clamp_u64(uint64_t value, uint64_t minimum, uint64_t maximum) {
    if (value < minimum) {
        return minimum;
    }
    if (value > maximum) {
        return maximum;
    }
    return value;
}

/* Choose a message count so the total bytes transferred are reasonable. */
uint64_t choose_message_count(size_t message_size) {
    /*
     * We keep the total transferred payload in a practical range so tiny
     * messages still run long enough to measure, while large messages do not
     * take too long. Three warm-up rounds are enough to stabilize TCP and the
     * scheduler; five measured rounds give us a repeatable peak sample.
     */
    const uint64_t target_bytes = 8ULL * 1024ULL * 1024ULL;
    uint64_t count = target_bytes / (uint64_t)message_size;
    return clamp_u64(count, 16ULL, 131072ULL);
}

/* Compute total bytes for the given message size and count. */
uint64_t bytes_to_human_count(size_t message_size, uint64_t message_count) {
    return (uint64_t)message_size * message_count;
}

/* Convert 64-bit host->network byte order. */
uint64_t htonll(uint64_t value) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return swap_u64(value);
#else
    return value;
#endif
}

/* Convert 64-bit network->host byte order. */
uint64_t ntohll(uint64_t value) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return swap_u64(value);
#else
    return value;
#endif
}

/* Configure common socket options (reuse address and larger buffers). */
static int set_common_socket_options(int socket_fd) {
    int enable = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) {
        return -1;
    }

    int buffer_size = 4 * 1024 * 1024;
    (void)setsockopt(socket_fd, SOL_SOCKET, SO_RCVBUF, &buffer_size, sizeof(buffer_size));
    (void)setsockopt(socket_fd, SOL_SOCKET, SO_SNDBUF, &buffer_size, sizeof(buffer_size));

    return 0;
}

/* Create, bind and listen on a TCP socket on the given port. */
int create_listening_socket(uint16_t port) {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        return -1;
    }

    if (set_common_socket_options(socket_fd) < 0) {
        perror("setsockopt");
        close(socket_fd);
        return -1;
    }

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);

    if (bind(socket_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind");
        close(socket_fd);
        return -1;
    }

    if (listen(socket_fd, 1) < 0) {
        perror("listen");
        close(socket_fd);
        return -1;
    }

    return socket_fd;
}

/* Resolve and connect to the specified host:port, returning a socket fd. */
int connect_to_host(const char *host, uint16_t port) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    char port_string[16];
    snprintf(port_string, sizeof(port_string), "%u", (unsigned)port);

    struct addrinfo *result = NULL;
    int error = getaddrinfo(host, port_string, &hints, &result);
    if (error != 0) {
        fprintf(stderr, "getaddrinfo(%s): %s\n", host, gai_strerror(error));
        return -1;
    }

    int socket_fd = -1;
    for (struct addrinfo *entry = result; entry != NULL; entry = entry->ai_next) {
        socket_fd = socket(entry->ai_family, entry->ai_socktype, entry->ai_protocol);
        if (socket_fd < 0) {
            continue;
        }

        if (set_common_socket_options(socket_fd) < 0) {
            close(socket_fd);
            socket_fd = -1;
            continue;
        }

        if (connect(socket_fd, entry->ai_addr, entry->ai_addrlen) == 0) {
            break;
        }

        close(socket_fd);
        socket_fd = -1;
    }

    freeaddrinfo(result);

    if (socket_fd < 0) {
        perror("connect");
        return -1;
    }

    return socket_fd;
}

/* Send the full buffer by repeatedly calling send until complete. */
int send_all(int socket_fd, const void *buffer, size_t length) {
    const unsigned char *cursor = (const unsigned char *)buffer;
    size_t sent = 0;

    while (sent < length) {
        ssize_t result = send(socket_fd, cursor + sent, length - sent, 0);
        if (result < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (result == 0) {
            return -1;
        }
        sent += (size_t)result;
    }

    return 0;
}

/* Receive the full buffer by repeatedly calling recv until complete. */
int recv_all(int socket_fd, void *buffer, size_t length) {
    unsigned char *cursor = (unsigned char *)buffer;
    size_t received = 0;

    while (received < length) {
        ssize_t result = recv(socket_fd, cursor + received, length - received, 0);
        if (result < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (result == 0) {
            return -1;
        }
        received += (size_t)result;
    }

    return 0;
}

/* Print usage information for the server executable. */
void print_usage_server(const char *program_name) {
    fprintf(stderr, "Usage: %s [port]\n", program_name);
}

/* Print usage information for the client executable. */
void print_usage_client(const char *program_name) {
    fprintf(stderr, "Usage: %s <server-host> [port]\n", program_name);
}