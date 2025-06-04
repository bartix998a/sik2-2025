#include <unistd.h>
#include <cerrno>
#include <cinttypes>
#include <cstdlib>
#include <arpa/inet.h>
#include <cstring>
#include <string>

#include "protocol_server.h"
#include "err.h"
#include "common.h"
#include "game.h"

static constexpr uint16_t K_MAX = 10000;
static constexpr uint16_t N_MAX = 8;
static constexpr long M_MAX = 12341234;



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

    return 0;
}