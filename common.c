#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <netdb.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "err.h"
#include "common.h"
// Writes the standard wrong args error messagge for the server/client
void wrong_args_client() {
    fatal("usage: -u <player_id> -p <port> -s <server> -4 -6 -a");
}

void wrong_args_server() {
    fatal("usage: -p <port> -k <value> -n <value> -m <value> -f <file>");
}

// Reads port from string and termintaes program if port number is incorrect.
uint16_t read_port(char const *string) {
    char *endptr;
    errno = 0;
    unsigned long port = strtoul(string, &endptr, 10);
    if (errno != 0 || *endptr != 0 || port > UINT16_MAX) {
        fatal("%s is not a valid port number", string);
    }
    return (uint16_t) port;
}
// Returns socket connected to server with name host and service port. ai_family specifies ip used.
int get_server_address(char const *host, char* port, int ai_family) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = ai_family;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo *address_result;
    int errcode = getaddrinfo(host, port, &hints, &address_result);
    if (errcode != 0) {
        fatal("getaddrinfo: %s", gai_strerror(errcode));
    }

    int socket_fd = socket(address_result->ai_family, address_result->ai_socktype, address_result->ai_protocol);
    if (socket_fd < 0) {
        syserr("cannot create a socket");
    }

    if (connect(socket_fd, address_result->ai_addr, address_result->ai_addrlen) < 0) {
        syserr("cannot connect to the server");
    }
    freeaddrinfo(address_result);

    return socket_fd;
}

// Write n bytes to a descriptor.
ssize_t writen(int fd, const void *vptr, size_t n){
    ssize_t nleft, nwritten;
    const char *ptr;

    ptr = (const char*) vptr;               // Can't do pointer arithmetic on void*.
    nleft = n;
    while (nleft > 0) {
        if ((nwritten = write(fd, ptr, nleft)) <= 0)
            return nwritten;  // error

        nleft -= nwritten;
        ptr += nwritten;
    }
    return n;
}
