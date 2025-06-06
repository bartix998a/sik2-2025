#include <unistd.h>
#include <cerrno>
#include <cinttypes>
#include <cstdlib>
#include <arpa/inet.h>
#include <cstring>
#include <string>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>

#include "protocol_server.h"
#include "err.h"
#include "common.h"
#include "game.h"
#include "debug.h"

static constexpr uint16_t K_MAX = 10000;
static constexpr uint16_t N_MAX = 8;
static constexpr long M_MAX = 12341234;

int get_socket(int port) {
    int socket_fd = socket(AF_INET6, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        syserr("cannot create a socket");
    }

    int opt = 0;
    setsockopt(socket_fd, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt));

    // Bind the socket to a concrete address.
    struct sockaddr_in6 server_address;
    server_address.sin6_family = AF_INET6; // IPv4
    server_address.sin6_addr = in6addr_any; // Listening on all interfaces.
    server_address.sin6_port = htons(port);

    if (bind(socket_fd, (struct sockaddr *) &server_address, (socklen_t) sizeof server_address) < 0) {
        syserr("bind");
    }

    // Switch the socket to listening.
    if (listen(socket_fd, 100) < 0) {
        syserr("listen");
    }

    if constexpr (debug) {
        std::cerr << "listening on port " << port << std::endl;
    }

    return socket_fd;
}

int main(int argc, char* argv[]) {
    int opt;
    uint16_t  port = 0;
    std::string file_path = "";
    while ((opt = getopt(argc, argv, "p:k:n:m:f:")) != -1) {
        switch (opt) {
            case 'p':
                port = read_port(optarg);
                break;
            case 'k': {
                long val = std::strtol(optarg, nullptr, 10);
                if (val < 1 || val > K_MAX) {
                    wrong_args();
                }
                K = val;
                break;
            }
            case 'n':
            {
                long val = std::strtol(optarg, nullptr, 10);
                if (val < 1 || val > N_MAX) {
                    wrong_args();
                }
                N = val;
                break;
            }
            case 'm':
            {
                long val = std::strtol(optarg, nullptr, 10);
                if (val < 1 || val > M_MAX) {
                    wrong_args();
                }
                M = val;
                break;
            }
            case 'f':
                file_path = optarg;
                break;
            default:
                wrong_args();
                return 1;
        }
    }

    if (file_path == "") {
        wrong_args();
    }

    run_server(get_socket(port), file_path);

    return 0;
}