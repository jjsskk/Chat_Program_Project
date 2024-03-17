#include <cstdlib>
#include <iostream>
#include <list>
#include "chat_room_server.h"
#include "chat_session_server.h"
#include "chat_server.h"

using namespace std;

int main(int argc, char *argv[])
{
  try
  {
    if (argc < 2)
    {
      cerr << "Usage: ChatServer <port> [<port> ...]\n";
      return 1;
    }

    boost::asio::io_service io_service;

    list<ChatServer> servers;
    for (int i = 1; i < argc; ++i)
    {
      tcp::endpoint endpoint(tcp::v4(), atoi(argv[i]));
      servers.emplace_back(io_service, endpoint, argv[i]);
    }

    io_service.run();
  }
  catch (exception &e)
  {
    cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}