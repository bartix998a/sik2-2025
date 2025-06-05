//
// Created by Bartek on 24.05.2025.
//

#ifndef ZAL1_CLIENT_DATA_H
#define ZAL1_CLIENT_DATA_H
#include <vector>
#include <poll.h>
#include <map>
#include <string>

extern std::vector<struct pollfd> poll_descriptors;

void add_player(int client_fd);
void add_send(int client_fd, int delay, const std::string& meessage);
void remove_client(int client_fd);
void execute_tasks();
bool answering(int client_fd);
int get_timeout();
void clear_tasks();
void handle_hello(int client_fd);

#endif //ZAL1_CLIENT_DATA_H
