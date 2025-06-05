//
// Created by Bartek on 24.05.2025.
//
#include <map>
#include <memory>
#include <string>
#include <sstream>
#include <iostream>
#include <poll.h>
#include <algorithm>
#include <format>
#include <limits>
#include <numbers>
#include <iomanip>

#include "common.h"
#include "common2.h"
#include "game.h"
#include "reading.h"
#include "client_data.h"

static std::map<int, std::string> ids;
static std::map<int, std::vector<double>> approximations;
static std::map<int, int> penalties;
static std::map<int, int> client_puts;

extern std::vector<struct pollfd> poll_descriptors;
int puts_count = 0;
u_int16_t K = 100;
u_int8_t N = 4;
long M = 131;

void add_player_score(int client_fd, const std::string& player_id) {
    ids[client_fd] = player_id;
    approximations[client_fd] = std::vector<double>(K, 0);
    penalties[client_fd] = 0;
    client_puts[client_fd] = 0;
}

std::string get_state(int client_fd) {
    std::ostringstream ss;
    ss << "STATE" << std::fixed << std::setprecision(7);
    for (auto el : approximations[client_fd]) {
        ss << " " << el;
    }
    ss << "\r\n";
    return ss.str();
}

void add_penalty(int client_fd, const std::string& msg) {
    auto tmp = "PENALTY " + msg.substr(4, msg.size() - 1);
    writen(client_fd, tmp.data(), tmp.size());
    penalties[client_fd] += 20;
}

void add_put(int client_fd, const std::string& msg) {
    puts_count++;
    approximations[client_fd][std::strtol(split(msg, ' ')[1].data(), nullptr, 10)] +=
            std::strtold(split(msg, ' ')[2].data(), nullptr);
    auto tmp = get_state(client_fd);
    auto delay = std::count_if(ids[client_fd].begin(), ids[client_fd].end(),
                               [](unsigned char c) {return std::islower(c);});
    add_send(client_fd, delay, tmp);
    add_send(STDOUT_FILENO, delay, "Sending state " + tmp.substr(6, tmp.size() - 2) + " to " + ids[client_fd] + "\n");
    client_puts[client_fd]++;
}

bool checkPutVals(const std::string& msg) {
    auto sp = split(msg, ' ');
    return std::strtol(sp[1].data(), nullptr, 10) <= K &&
           0 <= std::strtol(sp[1].data(), nullptr, 10) &&
           -5 <= std::strtol(sp[2].data(), nullptr, 10) &&
           std::strtol(sp[2].data(), nullptr, 10) <= 5;
}

// TODO: check if disconnected puts are counted towards M
void remove_client_score(int client_fd) {
    M -= client_puts[client_fd];
    ids.erase(client_fd);
    approximations.erase(client_fd);
    penalties.erase(client_fd);
}

void send_scoring() {

}

void clear_game() {
    ids.clear();
    approximations.clear();
    penalties.clear();
}

std::string escapeWhitespace(const std::string& input) {
    std::string output;
    for (char c : input) {
        switch (c) {
            case ' ':
                output += "\\s";  // or "\\ ", if you prefer
                break;
            case '\t':
                output += "\\t";
                break;
            case '\n':
                output += "\\n";
                break;
            case '\r':
                output += "\\r";
                break;
            default:
                output += c;
        }
    }
    return output;
}

void handle_wrong_message(int fd, std::string& msg) {
    std::cerr << "ERROR: bad message from " << print_ip_info(fd)
              << ", " << (ids.contains(fd) ? ids[fd] : "UNKNOWN") << ": "
              << msg.substr(0, msg.size()-2) << std::endl;
    if (!ids.contains(fd)) {
        remove_client(fd);
        remove_client_score(fd);
    }
}