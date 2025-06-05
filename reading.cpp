//
// Created by Bartek on 18.05.2025.
//

#include <string>
#include <cstdlib>
#include <unistd.h>
#include <regex>
#include <iostream>

#include "reading.h"
#include "err.h"
#include "debug.h"

static std::string point_reg = "[0-9]+";
static std::string rational_reg = R"([-+]?\d+\.\d{7})";

std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(s);
    std::string item;

    while (std::getline(ss, item, delimiter)) {
        tokens.push_back(item);
    }

    return tokens;
}

std::string read_msg(int socket_fd) {
    char input;
    bool end = false;
    std::string res = "";
    int val;
    while ((val = read(socket_fd, &input, 1)) && !end) {
        if (val < 0) {
            syserr("read");
        }

        if (res.back() == '\r' && input == '\n') {
            break;
        }
        res += input;
    }

    if (val == 0) {
        return res;
    }

    res += input;

    if constexpr (debug) {
        std::cerr << "recieved message " << res << std::endl;
    }

    return res;
}

void moveline(int in, int out) {
    int read_out = 0;
    char c;
    char prev = '\000';
    while ((read_out = read(in, &c, 1)) == 1) {
        if (read_out < 0) {
            syserr("read");
        }

        if (write(out, &c, 1) != 1) {
            syserr("write");
            break;
        }
        if (c == '\n' && prev == '\r') {
            break; // stop after reading one line
        }
        prev = c;
    }
}

bool checkHello(const std::string& msg) {
    std::regex put_reg("HELLO [a-zA-Z0-9]*\r\n");
    return std::regex_match(msg, put_reg);
}

bool checkPut(const std::string& msg) {
    std::regex put_reg("PUT [a-zA-Z0-9]* " + point_reg + " " + rational_reg + "\r\n");
    return std::regex_match(msg, put_reg);
}

bool checkState(const std::string& msg) {
    std::regex state_reg("PUT( " + rational_reg + ")*\r\n");
    return std::regex_match(msg, state_reg);
}

bool checkCoeff(const std::string& msg) {
    std::regex coeff_reg("COEFF(\\s" + rational_reg + ")*\r\n");
    return std::regex_match(msg, coeff_reg);
}

bool checkPutPlayerInput(const std::string& msg) {
    std::regex put_reg(point_reg + " " + rational_reg + "\r\n");
    return std::regex_match(msg, put_reg);
}