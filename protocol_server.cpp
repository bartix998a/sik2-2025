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
#include <thread>

#include "protocol_server.h"
#include "err.h"
#include "common.h"
#include "reading.h"
#include "message.h"
#include "client_data.h"
#include "game.h"
#include "common2.h"

static socklen_t sockaddr_in_len = sizeof(sockaddr_in);


// Function handling logic for the server.
void run_server(int socket_fd, const std::string& file) {
    int coeffs = open(file.data(), O_RDONLY);

    // Initialization of pollfd structures.

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
            last_msg[client_fd] = None;
            poll_descriptors[0].revents = 0;
        }

        for (size_t i = 1; i < poll_descriptors.size(); i++) {
            if (poll_descriptors[i].revents & (POLLIN | POLLERR)) {
                std::string msg = read_msg(poll_descriptors[i].fd);
                if (checkHello(msg) && last_msg[poll_descriptors[i].fd] == None) {
                    std::cout << print_ip_info(poll_descriptors[i].fd) << " is now know as " << split(msg, ' ')[1];
                    last_msg[poll_descriptors[i].fd] = COEFF;
                    add_player_score(poll_descriptors[i].fd, split(msg, ' ')[1]);
                    handle_hello(poll_descriptors[i].fd);
                    send_coeffs(poll_descriptors[i].fd,coeffs);
                } else if (checkPut(msg)) {
                    if ((last_msg[poll_descriptors[i].fd] != COEFF && last_msg[poll_descriptors[i].fd] != STATE)
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
            clear_game();
            clear_tasks();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }


    }

}