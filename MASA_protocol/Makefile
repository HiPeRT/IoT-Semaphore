demo: server client

server: demo/server.cpp
	g++ -Wall -g -std=c++11 -I./cereal/include -I./include demo/server.cpp -o server -lpthread -lrt

client: demo/client.cpp
	g++ -Wall -g -std=c++11 -I./cereal/include -I./include demo/client.cpp -o client -lpthread -lrt

clean:
	rm server
	rm client