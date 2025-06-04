//
// Created by Bartek on 24.05.2025.
//

#ifndef ZAL1_GAME_H
#define ZAL1_GAME_H

#include <string>
#include <tuple>
#include <vector>

extern int puts_count;
extern u_int16_t K;
extern u_int8_t N;
extern long M;

void add_player_score(int client_fd, const std::string& player_id);
void add_penalty(int client_fd, const std::string& msg);
void add_put(int client_fd, const std::string& msg);
void clear_game();
bool checkPutVals(const std::string& msg);
void remove_client_score(int client_fd);
void handle_wrong_message(int fd, std::string& msg);

#endif //ZAL1_GAME_H
