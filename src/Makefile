all: server client

server: server.cc
	g++ -std=c++11 -O3 -o server server.cc -lrt -lpthread

client: client.cc
	g++ -std=c++11 -O3 -o client client.cc -lrt -lpthread

clean:
	rm server client *~ 2> /dev/null || :
