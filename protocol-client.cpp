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

std::vector<double> send_hello(int server_fd, const std::string& id) {
    auto tmp = "Hello " + id + "\r\n";
    writen(server_fd, tmp.data(), tmp.size());
    std::string  coeffs = read_msg(server_fd);

    while (!checkCoeff(coeffs)) {
        // TODO : write error msg description
        coeffs = read_msg(server_fd);
    }

    std::cout << "Recieved coefficients " << coeffs.substr(5, coeffs.size() - 2) << std::endl;
    auto coeffs_split = split(coeffs, ' ');
    std::vector<double> res;
    std::transform(coeffs_split.begin() + 1, coeffs_split.end(), std::back_inserter(res),
                   [](auto& s){return std::stod(s);});
    return res;
}

bool process_msg(int fd) {
    std::string msg = read_msg(fd);
    auto msg_factorized = split(msg, ' ');
    if (msg_factorized[0] == "SCORING") {
        std::cout << "Game end, scoring: " << msg.substr(5, msg.size() - 2) << std::endl;
        return true;
    } else if (msg_factorized[0] == "STATE" && checkState(msg)) {
        std::cout << "Recieved state " << msg.substr(5, msg.size() - 2) << std::endl;
    } else {
        std::cerr << "ERROR: bas message from " << print_ip_info(fd)
        << ", UNKNOWN: " << msg.substr(0, msg.size()-2) << std::endl;
    }
    return false;
}

int run_client_automatic(int server_fd, const std::string& id) {
    unsigned long K;
    auto coeffs = send_hello(server_fd, id);
    std::vector<double> vals(coeffs.size(), 0);
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

        if (process_msg(server_fd)) {
            return 0;
        }
    }
}

int run_client(int server_fd, const std::string& id) {
    pollfd poll_descriptors[2];
    poll_descriptors[0].fd = STDIN_FILENO;
    poll_descriptors[1].fd = server_fd;
    bool recieved_coeffs;

    for (int i = 0; i < 2; ++i) {
        poll_descriptors[i].revents = 0;
        poll_descriptors[i].events = POLLIN;
    }
    send_hello(server_fd, id);

    while (true) {
        if (poll(poll_descriptors, 2, 0) < 0) {
            syserr("poll");
        }

        if (recieved_coeffs && poll_descriptors[0].revents == POLLIN) {
            auto line = read_msg(STDIN_FILENO);
            if (checkPutPlayerInput(line)) {
                auto tmp = "PUT " + line;
                writen(poll_descriptors[1].fd, tmp.data(), tmp.size());
            } else {
                std::cout << "ERROR: invalid input line " << line;
            }
            poll_descriptors[0].revents = 0;
        }

        if (process_msg(server_fd)) {
            return 0;
        }
    }
}