CC=g++
CFLAGS=-d -Wall -g

all: client

client: client.cpp requests.cpp helpers.cpp buffer.cpp commands.cpp
	$(CC) -o client client.cpp buffer.cpp requests.cpp helpers.cpp commands.cpp -w

run: client
	./client

clean:
	rm -f *.o client
