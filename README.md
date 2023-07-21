# Chat_Program_Project

- This program is an chat program made with Boost Library based on cpp. 

- [Boost.Asio Overview](https://www.boost.org/doc/libs/1_81_0/doc/html/boost_asio/overview.html)

- A terminal program was produced 

- Ubuntu(Linux) environment is required. 

- Installation required : `g++(std=c++11)`, `boost library`

- [boost_1_81_0.tar.gz download](https://boostorg.jfrog.io/artifactory/main/release/1.81.0/source/boost_1_81_0.tar.gz)

- You can also use makefile to compile

- When a client connects to the server, the server displays a list of currently running chat rooms to the client.

- Client can enter and leave the chat room he or she want. At this time, the status of entering or exiting the chat room is displayed to other clients participating in the chat room.

- In addition, newly entered clients can receive information about which clients are joining the room. 

- A client can create a new chat room, and when a new room is created, a new room list is delivered to clients who have not yet entered the room

- A file held by any one client can be sent to all clients participating in the chat room. 

- When a file is uploaded to the server, the server asks the clients participating in the chat room whether they want to receive the file, and sends the file only to those who want to receive it.

- Clients can send and receive conversations while files are being transferred.

- The server will run based on Boost.Asio as a whole

- When uploading files to the server, I used thread, and when clients download file from server, I used only Boost.Asio without using thread.

- See the guideline for detailed explanations with screenshots.

- reference C++11 chat Examples

- [C++11 chat Examples](https://www.boost.org/doc/libs/1_78_0/doc/html/boost_asio/examples/cpp11_examples.html)


```
g++ -std=c++11 chat_server.cpp -o chat_server -lboost_system -pthread
```

```
g++ -std=c++11 chat_client.cpp -o chat_client -lboost_system -pthread
```

```
./chat_client <host> <port> <ID>
```

```
./chat_server <port>
```