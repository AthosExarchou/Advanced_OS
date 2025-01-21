#C compiler and flags
CC = gcc
CFLAGS = -Wall

#executable and source files
TARGET = main
SRCS = main.c

#default target to build the program
all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

#clean up compiled files
clean:
	rm -f $(TARGET)

#run the program with example arguments
run: $(TARGET)
	./$(TARGET) $(OUTPUT_FILE) $(NUM_PROCESSES)
