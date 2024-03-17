#include <cstdlib>
#include <iostream>
#include "chat_room_server.h"
#include "chat_session_server.h"
#include "chat_server.h"

using namespace std;

int main(int argc, char *argv[])
{
  try
  {
    if (argc != 2)
    {
      cerr << "Usage: ChatServer <port>\n";
      return 1;
    }

    boost::asio::io_service io_service;

    tcp::endpoint endpoint(tcp::v4(), atoi(argv[1]));
    ChatServer server(io_service, endpoint, argv[1]);


    io_service.run();
  }
  catch (exception &e)
  {
    cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}