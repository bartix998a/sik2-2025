# Compiler and flags
CXX := g++
CC := gcc
CXXFLAGS := -std=gnu++20 -Wall -Wextra
CFLAGS := -std=gnu11 -Wall -Wextra

# Source files
COMMON_SRCS := err.c common.c common2.cpp reading.cpp message.h

SERVER_SRCS := server_main.cpp client_data.cpp game.cpp protocol_server.cpp $(COMMON_SRCS)
CLIENT_SRCS := client_main.cpp protocol-client.cpp $(COMMON_SRCS)

# Output binaries
SERVER_BIN := approx-server
CLIENT_BIN := approx-client

# Default target
all: $(SERVER_BIN) $(CLIENT_BIN)

# Server build
$(SERVER_BIN): $(SERVER_SRCS)
	$(CXX) $(CXXFLAGS) $(SERVER_SRCS) -o $(SERVER_BIN)

# Client build
$(CLIENT_BIN): $(CLIENT_SRCS)
	$(CXX) $(CXXFLAGS) $(CLIENT_SRCS) -o $(CLIENT_BIN)

# Clean target
clean:
	rm -f $(SERVER_BIN) $(CLIENT_BIN)

.PHONY: all clean
