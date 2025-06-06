//
// Created by Bartek on 18.05.2025.
//

#ifndef ZAL1_READING_H
#define ZAL1_READING_H

#include <string>
#include <vector>
#include <tuple>

std::string read_msg(int socket_fd, bool reading_stdin = false);
bool checkHello(const std::string& msg);
bool checkPut(const std::string& msg);
bool checkCoeff(const std::string& msg);
bool checkState(const std::string& msg);
bool checkPutPlayerInput(const std::string& msg);
std::vector<std::string> split(const std::string& s, char delimiter);

#endif //ZAL1_READING_H
