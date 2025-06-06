cmake_minimum_required(VERSION 3.25)
project(zal1 CXX C)

set(CMAKE_CXX_FLAGS "-std=gnu++20 -Wall -Wextra")
set(CMAKE_C_FLAGS "-std=gnu11 -Wall -Wextra")

add_executable(approx-server server_main.cpp
        err.c
        common.c
        message.h
        client_data.cpp
        game.cpp
        reading.cpp
        protocol_server.cpp
        common2.cpp
)

add_executable(approx-client client_main.cpp
        err.c
        common.c
        reading.cpp
        message.h
        protocol-client.cpp
        protocol-client.h
        common2.cpp
)
