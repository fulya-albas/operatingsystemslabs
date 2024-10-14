# Makefile to compile the size_info program

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall

# Target executable
TARGET = size_info

# Compile and link the target
all: $(TARGET)

$(TARGET): size_info.o
	$(CC) -o $(TARGET) size_info.o

# Compile the C source file into an object file
size_info.o: size_info.c
	$(CC) $(CFLAGS) -c size_info.c

# Clean up object files and the executable
clean:
	rm -f *.o $(TARGET)
