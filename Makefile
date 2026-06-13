CC      = gcc
CFLAGS  = -Wall -Iinclude
SRC     = $(wildcard src/*.c)
TARGET  = unsrat

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

.PHONY: all clean