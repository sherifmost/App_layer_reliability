# The compiler used
CC = g++

# The compilation method used
CFLAGS = -c -Wall

all: my_client_stop_wait my_server_stop_wait my_client my_server

my_client_stop_wait: parser.o file.o client_stop_wait.o
	$(CC) parser.o file.o client_stop_wait.o -o my_client_stop_wait

my_server_stop_wait: parser.o file.o server_stop_wait.o
	$(CC) parser.o file.o server_stop_wait.o -o my_server_stop_wait

my_client: parser.o file.o client.o
	$(CC) parser.o file.o client.o -o my_client

my_server: parser.o file.o server.o
	$(CC) parser.o file.o server.o -o my_server

parser.o: parser.cpp
	$(CC) $(CFLAGS) parser.cpp

file.o: file.cpp
	$(CC) $(CFLAGS) file.cpp

server_stop_wait.o: server_stop_wait.cpp
	$(CC) $(CFLAGS) server_stop_wait.cpp

client_stop_wait.o: client_stop_wait.cpp
	$(CC) $(CFLAGS) client_stop_wait.cpp

server.o: server.cpp
	$(CC) $(CFLAGS) server.cpp

client.o: client.cpp
	$(CC) $(CFLAGS) client.cpp

clean:
	rm -rf *o my_server_stop_wait my_client_stop_wait my_client my_server