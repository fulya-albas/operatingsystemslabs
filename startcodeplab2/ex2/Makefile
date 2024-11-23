CC = gcc
CFLAGS = -g -Wall
TARGET = case_reversal

all: $(TARGET)

$(TARGET): main.c case_reversal.c
	$(CC) $(CFLAGS) main.c case_reversal.c -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

debug: $(TARGET)
	gdb ./$(TARGET)

clean:
	rm -f $(TARGET)
