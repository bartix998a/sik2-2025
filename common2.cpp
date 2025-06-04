//
// Created by Bartek on 26.05.2025.
//

#include <string>
#include <arpa/inet.h>

#include "err.h"

std::string print_ip_info(int fd) {
    struct sockaddr_in6 addr;
    socklen_t addrlen = std::max(sizeof(sockaddr_in), sizeof(sockaddr_in6));;
    if (getpeername(fd, (struct sockaddr*)&addr, &addrlen) < 0) {
        syserr("getpeername");
    }

    switch (((sockaddr*) &addr)->sa_family) {
        case AF_INET: {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *) &addr;
            char ipstr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(ipv4->sin_addr), ipstr, sizeof(ipstr));
            return std::string(ipstr) + ":" + std::to_string(ipv4->sin_port);
        }
        case AF_INET6: {
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *) &addr;
            char ipstr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET6, &(ipv6->sin6_addr), ipstr, sizeof(ipstr));
            return std::string(ipstr) + ":" + std::to_string(ipv6->sin6_port);
        }
        default:
            syserr("unknown protocol");
    }
}