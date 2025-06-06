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
    std::string port = "0";
    int ai_family = AF_UNSPEC;
    std::string id = "";
    std::string server_ip = "";
    bool autoamted = false;
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
                auto tmp = std::strtol(optarg, nullptr, 10);
                if(tmp > 65535 || tmp < 0) {
                    wrong_args();
                }
                port = optarg;
                break;
            }
            case '4':
            {
                ai_family = AF_INET;
                break;
            }
            case '6':
                ai_family = AF_INET6;
                break;
            case 'a':
                autoamted = true;
                break;
            default:
                wrong_args();
                return 1;
        }
    }

    if (server_ip.empty()) {
        wrong_args();
    }

    int socket_fd = get_server_address(server_ip.data(), port.data(), ai_family);

    return autoamted ? run_client_automatic(socket_fd, id) : run_client(socket_fd, id);
}