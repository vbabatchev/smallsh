# Makefile for smallsh shell program

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g -std=c99 -pedantic

# Target executable name
TARGET = smallsh

# Source files
SRC = main.c parser.c commands.c signals.c bg_process.c io.c

# Object files (automatically generated from source files)
OBJ = $(SRC:.c=.o)

# Header files
HEADERS = common.h parser.h commands.h signals.h bg_process.h io.h

# Default target
all: $(TARGET)

# Linking rule
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^
	@echo "Build successful! Run with ./$(TARGET)"

# Compilation pattern rule for object files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Individual dependencies (for clarity)
main.o: main.c common.h parser.h commands.h signals.h bg_process.h io.h
parser.o: parser.c parser.h common.h
commands.o: commands.c commands.h common.h parser.h bg_process.h io.h signals.h
signals.o: signals.c signals.h common.h
bg_process.o: bg_process.c bg_process.h common.h
io.o: io.c io.h common.h parser.h

# Clean up generated files
clean:
	rm -f $(OBJ) $(TARGET)
	@echo "Cleaned up build files"

# Run the program
run: $(TARGET)
	./$(TARGET)

# Generate memory leak report
memcheck: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET)

# Debug with gdb
debug: $(TARGET)
	gdb ./$(TARGET)

# Phony targets
.PHONY: all clean run memcheck debug
