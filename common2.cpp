//
// Created by Bartek on 26.05.2025.
//

#include <string>
#include <arpa/inet.h>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

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
    std::string host;
    std::string port;
    host.resize(NI_MAXHOST);
    port.resize(NI_MAXSERV);

    if (getnameinfo((struct sockaddr*)&addr, addrlen, host.data(), 100,
            port.data(), 100, NI_NUMERICHOST | NI_NUMERICSERV) < 0) {
        syserr("getnameinfo");
    }

    return (addr.sin6_family == AF_INET6 ? host : host.substr(7)) + ":" + port;
}