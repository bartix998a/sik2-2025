//
// Created by Bartek on 26.05.2025.
//

#ifndef ZAL1_PROTOCOL_CLIENT_H
#define ZAL1_PROTOCOL_CLIENT_H

int run_client_automatic(int server_fd, const std::string& id);

int run_client(int server_fd, const std::string& id);

#endif //ZAL1_PROTOCOL_CLIENT_H
