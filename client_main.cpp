#include <unistd.h>
#include <cerrno>
#include <cinttypes>
#include <cstdlib>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>
#include <string>

#include "common.h"
#include "err.h"
#include "protocol-client.h"

// TODO; required args
int main(int argc, char* argv[]) {
    int opt;
    uint16_t  port = 0;
    uint16_t ip_type = AF_UNSPEC;
    std::string id = "";
    std::string server_ip = "";
    bool autoamted = false;
    auto tmp = 0;
    struct sockaddr_in addr;
    //TODO: -4 or -6
    while ((opt = getopt(argc, argv, "u:s:p:46a")) != -1) {
        switch (opt) {
            case 'u':
                id = optarg;
                break;
            case 's': {
                server_ip = optarg;
                break;
            }
            case 'p':
            {
                port = std::strtol(optarg, nullptr, 10);
                break;
            }
            case '4':
            {
                ip_type = AF_INET;
                break;
            }
            case '6':
                ip_type = AF_INET6;
                break;
            case 'a':
                autoamted = true;
                break;
            default:
                wrong_args();
                return 1;
        }
    }

    //TODO: ipv6
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        syserr("cannot create a socket");
    }

    addr = get_server_address(server_ip.data(), port);

    if (connect(socket_fd, (struct sockaddr *) &addr,
                (socklen_t) sizeof(addr)) < 0) {
        syserr("cannot connect to the server");
    }

    return autoamted ? run_client_automatic(socket_fd, id) : run_client(socket_fd, id);
}