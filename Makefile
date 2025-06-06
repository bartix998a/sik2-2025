# Compilers
CC = gcc
CXX = g++

# Compiler flags
CFLAGS = -std=gnu11 -Wall -Wextra
CXXFLAGS = -std=gnu++20 -Wall -Wextra

# Sources
COMMON_C_SRCS = err.c common.c common2.c reading.cpp
SERVER_SRCS = server_main.cpp client_data.cpp game.cpp protocol_server.cpp $(COMMON_C_SRCS)
CLIENT_SRCS = client_main.cpp protocol-client.cpp $(COMMON_C_SRCS)

# Executables
SERVER_BIN = approx-server
CLIENT_BIN = approx-client

# Object directories
OBJDIR = obj
SRCDIRS = .

# Convert .c and .cpp files to .o
SERVER_OBJS = $(addprefix $(OBJDIR)/, $(SERVER_SRCS:.c=.o))
SERVER_OBJS := $(SERVER_OBJS:.cpp=.o)

CLIENT_OBJS = $(addprefix $(OBJDIR)/, $(CLIENT_SRCS:.c=.o))
CLIENT_OBJS := $(CLIENT_OBJS:.cpp=.o)

# Default target
all: $(SERVER_BIN) $(CLIENT_BIN)

# Build approx-server
$(SERVER_BIN): $(SERVER_OBJS)
	$(CXX) $(SERVER_OBJS) -o $@

# Build approx-client
$(CLIENT_BIN): $(CLIENT_OBJS)
	$(CXX) $(CLIENT_OBJS) -o $@

# Compile C files
$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile C++ files
$(OBJDIR)/%.o: %.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Create obj directory if not exists
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Clean build artifacts
clean:
	rm -rf $(OBJDIR) $(SERVER_BIN) $(CLIENT_BIN)

.PHONY: all clean
