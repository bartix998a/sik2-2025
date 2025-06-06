#include "client_data.h"


#include <chrono>
#include <set>
#include <memory>
#include <poll.h>
#include <map>
#include <algorithm>
#include <cmath>

#include "common.h"
#include "common2.h"


/*
 * This set of functions ensure that poll terminates with timeout only when there is something to be done.
 * To ensure this we keep track of all actions to be executed with some delay in the set tasks. In tasks
 * the elements are ordered lexicographically, the first element of the tuple is the time of execution of the task.
 * Appropriately named functions handle adding and removing clients as well as timeouts related to hello.
 * */

enum task {
    WAIT_FOR_HELLO,
    SEND_WITH_DELAY,
    REMOVE
};

using system_clock = std::chrono::system_clock;
using miliseconds = std::chrono::milliseconds;
using seconds = std::chrono::seconds;

// In all the below maps the key is the file descriptor of the client.
static std::set<std::tuple<system_clock::time_point, task, std::shared_ptr<std::string>, int>> tasks;
std::vector<struct pollfd> poll_descriptors(1);
std::map<int, Message> last_msg;
std::map<int, std::string> ids;

void add_player(int client_fd) {
    poll_descriptors.push_back({client_fd, POLLIN, 0});
    tasks.insert(std::tuple(system_clock::now() + miliseconds(3000), WAIT_FOR_HELLO, nullptr, client_fd));
}

void add_send(int client_fd, int delay, const std::string &meessage) {
    tasks.insert(std::tuple(system_clock::now() + seconds(delay),
                            SEND_WITH_DELAY, std::make_shared<std::string>(meessage), client_fd));
}

// Removing client while iterating through poll_descriptor could cause bugs. This function safely removes a client after
// the iteration is done. The client is removed upon the next call of execute_tasks.
void remove_client(int client_fd) {
    close(client_fd);
    erase_if(tasks, [client_fd](auto t) { return std::get<3>(t) == client_fd; });
    tasks.insert(std::tuple(system_clock::now(), REMOVE, nullptr, client_fd));
}

void handle_hello(int client_fd) {
    erase_if(tasks, [client_fd](auto t) { return std::get<3>(t) == client_fd && get<1>(t) == WAIT_FOR_HELLO; });
}

// Executes all tasks scheduled to execute earlier than or at the moment the function was called.
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
                    last_msg.erase(get<3>(task));
                    break;
            }
        }
        tasks.erase(tasks.begin());
    }
}

// true iff the server is still waiting to answer to the client with client_fd.
bool answering(int client_fd) {
    return std::find_if(
            tasks.begin(), tasks.end(),
            [client_fd](auto &t) { return std::get<3>(t) == client_fd && get<1>(t) == SEND_WITH_DELAY; }) != tasks.end();
}

int get_timeout() {
    // abs() is here to make sure that the timeout on non empty tasks is always positive as timeouts smaller than -1 cause undefined
    // behaviour and -1 makes the poll never timeout. If the duration size is negative then the timeout should be small,
    // therefore this won't cause an issue.
    return tasks.empty() ? -1 : std::chrono::abs(std::chrono::duration_cast<miliseconds>(
            std::get<0>(*std::min_element(tasks.begin(), tasks.end())) - system_clock::now())).count();
}

// Clears all task related data for all clients.
void clear_tasks() {
    tasks.clear();
    for (size_t i = 1; i < poll_descriptors.size(); i++) {
        close(poll_descriptors[i].fd);
    }

    poll_descriptors.erase(++poll_descriptors.begin(), poll_descriptors.end());
    last_msg.clear();

}

