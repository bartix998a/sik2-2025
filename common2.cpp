//
// Created by Bartek on 26.05.2025.
//

#include <string>
#include <arpa/inet.h>
#include <vector>

#include "err.h"

// Evaluates a polynomial with coefficients written in coeffs in the point K.
double eval(const std::vector<double>& coeffs, int K) {
    double result = 0;
    int power = 1;  // K^0 initially

    for (auto coeff : coeffs) {
        result += coeff * power;
        power *= K;
    }

    return result;
}

// Returns a string of the form "<hostname>:<port>" of the other device connected to fd.
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
            return std::string(ipstr) + ":" + std::to_string(ntohs(ipv4->sin_port));
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