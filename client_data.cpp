#include "client_data.h"


#include <chrono>
#include <set>
#include <memory>
#include <poll.h>
#include <map>
#include <algorithm>
#include <iostream>

#include "common.h"
#include "common2.h"


enum task {
    WAIT_FOR_HELLO,
    SEND_WITH_DELAY,
    REMOVE
};

using system_clock = std::chrono::system_clock;
using miliseconds = std::chrono::milliseconds;
using seconds = std::chrono::seconds;

// key is socket descriptor
static std::set<std::tuple<system_clock::time_point, task, std::shared_ptr<std::string>, int>> tasks;
std::vector<struct pollfd> poll_descriptors(1);
std::map<int, std::string> ids;

void add_player(int client_fd) {
    poll_descriptors.push_back({client_fd, POLLIN, 0});
    tasks.insert(std::tuple(system_clock::now() + miliseconds(3000), WAIT_FOR_HELLO, nullptr, client_fd));
}

void add_send(int client_fd, int delay, const std::string &meessage) {
    tasks.insert(std::tuple(system_clock::now() + seconds(delay),
                            SEND_WITH_DELAY, std::make_shared<std::string>(meessage), client_fd));
}

void remove_client(int client_fd) {
    close(client_fd);
    tasks.insert(std::tuple(system_clock::now(), REMOVE, nullptr, client_fd));
    erase_if(tasks, [client_fd](auto t) { return std::get<3>(t) == client_fd; });
}

void handle_hello(int client_fd) {
    erase_if(tasks, [client_fd](auto t) { return std::get<3>(t) == client_fd && get<1>(t) == WAIT_FOR_HELLO; });
}

// might break cause you remove stuff inside switch statement
void execute_tasks() {
    auto now = system_clock::now();
    while (!tasks.empty() && std::get<0>(*tasks.begin()) < now) {
        auto task = *tasks.begin();

        if (std::get<0>(task) < now) {
            switch (std::get<1>(task)) {
                case WAIT_FOR_HELLO:
                    remove_client(std::get<3>(task));
                    break;
                case SEND_WITH_DELAY:
                    writen(std::get<3>(task), std::get<2>(task).get()->data(), std::get<2>(task)->size());
                    break;
                case REMOVE:
                    std::erase_if(poll_descriptors, [task](pollfd f) { return f.fd == get<3>(task); });
                    break;
            }
        }
        tasks.erase(tasks.begin());
    }
}

bool answering(int client_fd) {
    return std::find_if(
            tasks.begin(), tasks.end(),
            [client_fd](auto &t) { return std::get<3>(t) == client_fd; }) != tasks.end();
}

int get_timeout() {
    return tasks.empty() ? -1 : std::chrono::duration_cast<miliseconds>(
            std::get<0>(*std::min_element(tasks.begin(), tasks.end())) - system_clock::now()).count();
}

void clear_tasks() {
    tasks.clear();
    for (size_t i = 1; i < poll_descriptors.size(); i++) {
        close(poll_descriptors[i].fd);
    }

    poll_descriptors.erase(poll_descriptors.begin()++, poll_descriptors.end());

}

