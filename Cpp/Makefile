all: 
	g++ -std=c++11 chat_server.cpp -o chat_server -lboost_system -pthread
	g++ -std=c++11 chat_client.cpp -o chat_client -lboost_system -pthread

clean:
	rm chat_server 
	rm chat_client
	