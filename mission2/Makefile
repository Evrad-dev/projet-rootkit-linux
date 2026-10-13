
CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11
TARGET = companion

all: $(TARGET)

$(TARGET): companion.c
	$(CC) $(CFLAGS) -o $(TARGET) companion.c

clean:
	rm -f $(TARGET)

.PHONY: all clean
