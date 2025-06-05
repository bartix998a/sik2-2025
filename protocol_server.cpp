#include <cinttypes>
#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <vector>
#include <chrono>
#include <queue>
#include <fstream>
#include <iostream>

#include "debug.h"
#include "protocol_server.h"
#include "err.h"
#include "common.h"
#include "reading.h"
#include "message.h"
#include "client_data.h"
#include "game.h"
#include "common2.h"

static socklen_t sockaddr_in_len = sizeof(sockaddr_in);

int get_socket(int port) {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        syserr("cannot create a socket");
    }

    // Bind the socket to a concrete address.
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET; // IPv4
    server_address.sin_addr.s_addr = htonl(INADDR_ANY); // Listening on all interfaces.
    server_address.sin_port = htons(port);

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

// TODO: add last_msg to client_data
// TODO: disconect upon no hello probably causes segfault
void run_server(int port, const std::string& file) {
    int socket_fd = get_socket(port);
    int coeffs = open(file.data(), O_RDONLY);

    // Initialization of pollfd structures.

    std::vector<Message> last_msg(1); // last message sent by server to client

    // The main socket has index 0.
    poll_descriptors[0].fd = socket_fd;
    poll_descriptors[0].revents = 0;
    poll_descriptors[0].events = POLLIN;

    while (true) {
        struct sockaddr_in client_address;
        for (auto p : poll_descriptors) {
            p.revents = 0;
        }

        if (poll(poll_descriptors.data(), poll_descriptors.size(),
                 get_timeout()) < 0) {
            syserr("poll");
        }

        if (poll_descriptors[0].revents == POLLIN) {
            int client_fd = accept(socket_fd, (struct sockaddr *) &client_address, &sockaddr_in_len);
            if (client_fd < 0) {
                syserr("accept");
            }

            // Set to nonblocking mode.
            if (fcntl(client_fd, F_SETFL, O_NONBLOCK)) {
                syserr("fcntl");
            }
            add_player(client_fd);
            last_msg.push_back(None);
            poll_descriptors[0].revents = 0;
        }

        for (size_t i = 1; i < poll_descriptors.size(); i++) {
            if (poll_descriptors[i].revents & (POLLIN | POLLERR)) {
                std::string msg = read_msg(poll_descriptors[i].fd);
                if (checkHello(msg) && last_msg[i] == None) {
                    std::cout << print_ip_info(poll_descriptors[i].fd) << " is now know as " << split(msg, ' ')[1];
                    last_msg[i] = COEFF;
                    add_player_score(poll_descriptors[i].fd, split(msg, ' ')[1]);
                    handle_hello(poll_descriptors[i].fd);
                    send_coeffs(poll_descriptors[i].fd,coeffs);
                } else if (checkPut(msg)) {
                    if ((last_msg[i] != COEFF && last_msg[i] != STATE)
                        || answering(poll_descriptors[i].fd)) {
                        add_penalty(poll_descriptors[i].fd, msg);
                    } else if (!checkPutVals(msg)) {
                        auto tmp = "BAD PUT " + msg.substr(4, msg.size() - 1);
                        add_send(poll_descriptors[i].fd, 1, tmp);
                    } else {
                        add_put(poll_descriptors[i].fd, msg);
                    }
                } else if (msg == "") { // client disconnects
                    remove_client((poll_descriptors[i].fd));
                    remove_client_score(poll_descriptors[i].fd);
                } else {
                    handle_wrong_message(poll_descriptors[i].fd, msg);
                }
                poll_descriptors[i].revents = 0;
            } else if (poll_descriptors[i].revents & POLLHUP) {
                remove_client_score(poll_descriptors[i].fd);
                remove_client(poll_descriptors[i].fd);
            }

            if (puts_count == M) {
                break;
            }
        }

        if (puts_count < M) {
            execute_tasks();
        } else {
            send_scoring();
            std::cout << "post scoring alive\n";
//            clear_game();
//            clear_tasks();
        }


    }

}