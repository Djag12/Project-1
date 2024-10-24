# Define variables
CC = gcc
CFLAGS = -Wall -g -std=c99
TARGET = shell
SRCS = shell.c

# Default target
all: $(TARGET)

# Rule for building the shell executable
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

# Rule for cleaning up
clean:
	rm -f $(TARGET)
