//
// Created by Bartek on 26.05.2025.
//

#include <iostream>
#include <unistd.h>
#include <poll.h>
#include <string>
#include <cstdlib>
#include <cmath>
#include <sstream>
#include <functional>
#include <numeric>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <climits>

#include "err.h"
#include "protocol-client.h"
#include "reading.h"
#include "common.h"
#include "common2.h"


template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

void print_bad_msg(int fd, const std::string& msg) {
    std::cerr << "ERROR: bad message from " << print_ip_info(fd)
    << ", UNKNOWN: " << msg.substr(0, msg.size()-2) << std::endl;
}

std::vector<double> send_hello(int server_fd, const std::string& id) {
    auto tmp = "HELLO " + id + "\r\n";
    writen(server_fd, tmp.data(), tmp.size());
    std::string  coeffs = read_msg(server_fd);

        if (coeffs == "") {
            std::cerr << "ERROR: unexpected server disconnect" << std::endl;
            return {};
        }

        if (!checkCoeff(coeffs)) {
            print_bad_msg(server_fd, coeffs);
            return {};
        }


    std::cout << "Recieved coefficients " << coeffs.substr(5, coeffs.size());
    auto coeffs_split = split(coeffs, ' ');
    std::vector<double> res;
    std::transform(coeffs_split.begin() + 1, coeffs_split.end(), std::back_inserter(res),
                   [](auto& s){return std::stod(s);});
    return res;
}

int process_msg(int fd, int& ret) {
    std::string msg = read_msg(fd);

    if (msg == "") {
        std::cerr << "ERROR: unexpected server disconnect" << std::endl;
        ret = 1;
        return 0;
    }

    auto msg_factorized = split(msg, ' ');
    if (msg_factorized[0] == "SCORING") {
        std::cout << "Game end, scoring: " << msg.substr(8, msg.size() - 2) << std::endl;
        ret = 0;
        return 0;
    } else if (msg_factorized[0] == "STATE" && checkState(msg)) {
        std::cout << "Recieved state " << msg.substr(5, msg.size());
    } else if (msg_factorized[0] == "COEFFS" && checkCoeff(msg)) {
        std::cout << "Recieved coefficients " << msg.substr(6);
    } else if (msg_factorized[0] == "BAD_PUT" && checkBadPut(msg)) {
        std::cout << "Bad put " << msg.substr(7);
        return 2;
    } else {
        print_bad_msg(fd, msg);
    }
    return 1;
}

int run_client_automatic(int server_fd, const std::string& id) {
    auto coeffs = send_hello(server_fd, id);
    int current_put = 0;
    int K = INT_MAX;

    if (coeffs.empty()) {
        return 1;
    }
    std::vector<double> vals(0);
    int ret;
    while (true) {
        bool found = false;
        std::cout << "restarted loop" << std::endl;
        for (size_t i = 0; i < K; i++) {
            if (vals.size() == i) {
                vals.push_back(0.);
            }
            auto diff = eval(coeffs, i) - vals[i];
            if (std::fabs(diff) > 0.0001) {
                found = true;
                auto value = sgn(diff) * std::min(fabs(eval(coeffs, i) - vals[i]), 5.);
                vals[i] += value;
                std::ostringstream ss;
                ss << "PUT " << i << " " << value << "\r\n";
                std::cout << "Putting " << value << " in " << i << std::endl;
                writen(server_fd, ss.str().data(), ss.str().size());
                break;
            }
        }

        if (!found) {
            auto value = 0.;
            vals[0] += value;
            std::ostringstream ss;
            ss << "PUT " << 0 << " " << value << "\r\n";
            std::cout << "Putting " << value << " in " << 0 << std::endl;
            writen(server_fd, ss.str().data(), ss.str().size());
        }

        if ((current_put = process_msg(server_fd, ret)) > 0) {
            return ret;
        }

        if (current_put == 2) {
            K = current_put;
        }
    }
}

int run_client(int server_fd, const std::string& id) {
    pollfd poll_descriptors[2];
    poll_descriptors[0].fd = STDIN_FILENO;
    poll_descriptors[1].fd = server_fd;
    int ret;

    for (int i = 0; i < 2; ++i) {
        poll_descriptors[i].revents = 0;
        poll_descriptors[i].events = POLLIN;
    }
    if (send_hello(server_fd, id).empty()) {
        close(server_fd);
        return 1;
    }

    if (fcntl(server_fd, F_SETFL, O_NONBLOCK)) {
        syserr("fcntl");
    }

    while (true) {
        if (poll(poll_descriptors, 2, -1) < 0) {
            syserr("poll");
        }

        if (poll_descriptors[0].revents == POLLIN) {
            auto line = read_msg(STDIN_FILENO, true);
            if (checkPutPlayerInput(line)) {
                auto tmp = "PUT " + line.substr(0, line.size() - 1) + "\r\n";
                writen(poll_descriptors[1].fd, tmp.data(), tmp.size());
                std::cout << "Putting " << split(line, ' ')[0] << " in " << split(line, ' ')[1];
            } else {
                std::cout << "ERROR: invalid input line " << line;
            }
            poll_descriptors[0].revents = 0;
        }

        if (poll_descriptors[1].revents == POLLIN && process_msg(server_fd, ret)) {
            return ret;
        }
    }
}