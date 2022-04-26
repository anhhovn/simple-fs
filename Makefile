#the compiler: gcc for C program
CC = gcc


# compiler flags:
# -g	adds debugging information to the executable file
# -Wall turns on most, but not all, compiler warnings
CFLAGS = -g -Wall
OBJFILES = fs.o disk.o test.o
# the build target executable
TARGET = test

all: $(TARGET)

$(TARGET): $(OBJFILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJFILES) 

clean:
	rm -f $(OBJFILES) $(TARGET) *~