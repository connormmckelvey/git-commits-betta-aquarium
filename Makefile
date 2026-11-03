CC = gcc
CFLAGS = -O2 -Wall -Wextra
LIBS = -lm

SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)
TARGET = git-betta

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LIBS)

clean:
	rm -f $(TARGET) src/*.o *.exe

.PHONY: all clean
