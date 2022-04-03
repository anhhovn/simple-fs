#the compiler: gcc for C program
CC = gcc

# compiler flags:
# -g	adds debugging information to the executable file
# -Wall turns on most, but not all, compiler warnings
CFLAGS = -g -Wall

# the build target executable
TARGET = disk

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -c -o $(TARGET) $(TARGET).c

clean:
	$(RM) $(TARGET)