
#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <utility>
#include <boost/asio.hpp>
#include "globalVariable.h"

using namespace std;

#include "chat_room_server.h"
#include "chat_session_server.h"

class ChatServer
{
public:
  ChatServer(boost::asio::io_service &io_service,
              const tcp::endpoint &endpoint, string port)
      : io_service_(io_service), acceptor_(io_service, endpoint),
        socket_(io_service), port_(port)
  {
    do_accept();
  }

private:
  void do_accept()
  {
    acceptor_.async_accept(socket_,
                           [this](boost::system::error_code ec)
                           {
                             if (!ec)
                             {
                               shared_ptr<ChatSession> ptr = make_shared<ChatSession>(io_service_, move(socket_), roomlist_, port_);
                               ptr->Start();
                               participants_life_.insert(ptr);
                             }

                             do_accept();
                           });
  }
  boost::asio::io_service &io_service_;
  tcp::acceptor acceptor_;
  tcp::socket socket_;
  list<shared_ptr<ChatRoom>> roomlist_;
  string port_;
};

//----------------------------------------------------------------------

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