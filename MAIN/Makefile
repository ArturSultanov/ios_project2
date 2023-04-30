CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Werror -pedantic
LDFLAGS = -lrt -lpthread
TARGET = proj2

all: $(TARGET)

$(TARGET): proj2.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) proj2.o

proj2.o: proj2.c
	$(CC) $(CFLAGS) -c proj2.c

clean:
	rm -f *.o $(TARGET)
