# Cross-platform Makefile for Notepad application
# Works on Linux, macOS (with Homebrew), and Windows (with MSYS2/MinGW)

CC ?= gcc
CFLAGS := $(shell pkg-config --cflags gtk+-3.0) -Wall -Wextra -O2
LIBS := $(shell pkg-config --libs gtk+-3.0)

TARGET = notepad
SRC = notepad.c
OBJ = notepad.o

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LIBS)

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $(SRC) -o $(OBJ)

clean:
	rm -f $(OBJ) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
