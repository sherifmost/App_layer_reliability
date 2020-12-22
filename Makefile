# The compiler used
CC = g++

# The compilation method used
CFLAGS = -c -Wall

all: my_client_stop_wait my_server_stop_wait

my_client_stop_wait: parser.o file.o client_stop_wait.o
	$(CC) parser.o file.o client_stop_wait.o -o my_client_stop_wait

my_server_stop_wait: parser.o file.o server_stop_wait.o
	$(CC) parser.o file.o server_stop_wait.o -o my_server_stop_wait


parser.o: parser.cpp
	$(CC) $(CFLAGS) parser.cpp

file.o: file.cpp
	$(CC) $(CFLAGS) file.cpp

server_stop_wait.o: server_stop_wait.cpp
	$(CC) $(CFLAGS) server_stop_wait.cpp

client_stop_wait.o: client_stop_wait.cpp
	$(CC) $(CFLAGS) client_stop_wait.cpp

clean:
	rm -rf *o my_server_stop_wait my_client_stop_wait