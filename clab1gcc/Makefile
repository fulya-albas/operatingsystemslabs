# Variables
CC = gcc               # The compiler to use
CFLAGS = -Wall -g     # Compiler flags: show all warnings and include debugging information
TARGET = size_of_types # Name of the output executable

# Default target
all: $(TARGET)

# Build the target
$(TARGET): main.o
	$(CC) $(CFLAGS) -o $(TARGET) main.o

# Compile the object file
main.o: main.c
	$(CC) $(CFLAGS) -c main.c

# Clean up
clean:
	rm -f *.o $(TARGET)
