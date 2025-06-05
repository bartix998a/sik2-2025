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

#include "err.h"
#include "protocol-client.h"
#include "reading.h"
#include "common.h"
#include "common2.h"

double eval(const std::vector<double>& coeffs, int K) {
    double result = 0;
    int power = 1;  // K^0 initially

    for (auto coeff : coeffs) {
        result += coeff * power;
        power *= K;
    }

    return result;
}

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

    while (true) {
        if (coeffs == "") {
            std::cerr << "ERROR: unexpected server disconnect" << std::endl;
            return {};
        }

        if (checkCoeff(coeffs)) {
            break;
        } else {
            std::cout << "here2" << std::endl;
            print_bad_msg(server_fd, coeffs);
            coeffs = read_msg(server_fd);
        }

    }

    std::cout << "Recieved coefficients " << coeffs.substr(5, coeffs.size() - 2) << std::endl;
    auto coeffs_split = split(coeffs, ' ');
    std::vector<double> res;
    std::transform(coeffs_split.begin() + 1, coeffs_split.end(), std::back_inserter(res),
                   [](auto& s){return std::stod(s);});
    return res;
}

bool process_msg(int fd, int& ret) {
    std::string msg = read_msg(fd);

    if (msg == "") {
        std::cerr << "ERROR: unexpected server disconnect" << std::endl;
        ret = 1;
        return true;
    }

    auto msg_factorized = split(msg, ' ');
    if (msg_factorized[0] == "SCORING") {
        std::cout << "Game end, scoring: " << msg.substr(5, msg.size() - 2) << std::endl;
        ret = 0;
        return true;
    } else if (msg_factorized[0] == "STATE" && checkState(msg)) {
        std::cout << "Recieved state " << msg.substr(5, msg.size() - 2) << std::endl;
    } else if (msg_factorized[0] == "COEFFS" && checkCoeff(msg)) {
        std::cout << "Recieved coefficients " << msg.substr(5, msg.size() - 2) << std::endl;
    } else {
        std::cout << "here" << std::endl;
        print_bad_msg(fd, msg);
    }
    return false;
}

int run_client_automatic(int server_fd, const std::string& id) {
    unsigned long K;
    auto coeffs = send_hello(server_fd, id);
    if (coeffs.empty()) {
        return 1;
    }
    std::vector<double> vals(coeffs.size(), 0);
    int ret;
    while (true) {
        for (size_t i = 0; i < coeffs.size(); i++) {
            auto diff = eval(coeffs, i) - vals[i];
            if (std::fabs(diff) > 0.0001) {
                auto value = sgn(diff) * std::min(fabs(eval(coeffs, i) - vals[i]), 5.);
                vals[i] += value;
                std::ostringstream ss;
                ss << "PUT " << i << " " << value << "\r\n";
                std::cout << "Putting " << value << " in " << i << std::endl;
                writen(server_fd, ss.str().data(), ss.str().size());
                break;
            }
        }

        if (process_msg(server_fd, ret)) {
            return ret;
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
        return 1;
    }

    while (true) {
        if (poll(poll_descriptors, 2, 0) < 0) {
            syserr("poll");
        }

        if (poll_descriptors[0].revents == POLLIN) {
            auto line = read_msg(STDIN_FILENO);
            if (checkPutPlayerInput(line)) {
                auto tmp = "PUT " + line;
                writen(poll_descriptors[1].fd, tmp.data(), tmp.size());
            } else {
                std::cout << "ERROR: invalid input line " << line;
            }
            poll_descriptors[0].revents = 0;
        }

        if (process_msg(server_fd, ret)) {
            return ret;
        }
    }
}