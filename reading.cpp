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

static std::string point_reg = "[0-9]+";
static std::string rational_reg = R"([-+]?\d+(\.\d{0,7}){0,1})";

// splits string by delimeter
std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(s);
    std::string item;

    while (std::getline(ss, item, delimiter)) {
        tokens.push_back(item);
    }

    return tokens;
}

// Read from socket_fd until \r\n sequence is encountered if reading_stdin = false or until \n otherwise
std::string read_msg(int socket_fd, bool reading_stdin) {
    char input;
    bool end = false;
    std::string res = "";
    int val;
    while ((val = read(socket_fd, &input, 1)) && !end) {
        if (val < 0) {
            syserr("read");
        }

        if ((res.back() == '\r' || reading_stdin) && input == '\n') {
            break;
        }
        res += input;
    }

    if (val == 0) {
        return res;
    }

    res += input;

    return res;
}

// The functions below verify correctness of appropriate messages.

bool checkHello(const std::string& msg) {
    std::regex put_reg("HELLO [a-zA-Z0-9]*\r\n");
    return std::regex_match(msg, put_reg);
}

bool checkPut(const std::string& msg) {
    std::regex put_reg("PUT " + point_reg + " " + rational_reg + "\r\n");
    return std::regex_match(msg, put_reg);
}

bool checkState(const std::string& msg) {
    std::regex state_reg("STATE( " + rational_reg + "){1,}\r\n");
    return std::regex_match(msg, state_reg);
}

bool checkCoeff(const std::string& msg) {
    std::regex coeff_reg("COEFF(\\s" + rational_reg + ")*\r\n");
    return std::regex_match(msg, coeff_reg);
}

bool checkPutPlayerInput(const std::string& msg) {
    std::regex put_reg(point_reg + "\\s" + rational_reg + "\n");
    return std::regex_match(msg, put_reg);
}