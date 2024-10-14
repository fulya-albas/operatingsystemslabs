# Makefile to compile both size_info and string_manipulation programs

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall

# Targets
TARGET1 = size_info
TARGET2 = strings

all: $(TARGET1) $(TARGET2)

# Compile size_info
$(TARGET1): size_info.o
	$(CC) -o $(TARGET1) size_info.o

size_info.o: size_info.c
	$(CC) $(CFLAGS) -c size_info.c

# Compile string_manipulation
$(TARGET2): strings.o
	$(CC) -o $(TARGET2) strings.o

string_manipulation.o: strings.c
	$(CC) $(CFLAGS) -c strings.c

# Clean up object files and executables
clean:
	rm -f *.o $(TARGET1) $(TARGET2)

