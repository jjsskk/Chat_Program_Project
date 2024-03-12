
#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <utility>
#include <boost/asio.hpp>
#include "globalVariable.h"

using boost::asio::ip::tcp;


#include "chat_room_server.h"
#include "chat_session_server.h"

class ChatServer
{
public:
  ChatServer(boost::asio::io_service &io_service,
              const tcp::endpoint &endpoint, std::string port)
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
                               //  participants_life_.insert(std::make_shared<ChatSession>(io_service_,std::move(socket_), roomlist_));
                               std::shared_ptr<ChatSession> ptr = std::make_shared<ChatSession>(io_service_, std::move(socket_), roomlist_, port_);
                               ptr->Start();
                               participants_life_.insert(ptr);
                             }

                             do_accept();
                           });
  }
  boost::asio::io_service &io_service_;
  tcp::acceptor acceptor_;
  tcp::socket socket_;
  std::list<std::shared_ptr<ChatRoom>> roomlist_;
  std::string port_;
};

//----------------------------------------------------------------------

int main(int argc, char *argv[])
{
  try
  {
    if (argc < 2)
    {
      std::cerr << "Usage: ChatServer <port> [<port> ...]\n";
      return 1;
    }

    boost::asio::io_service io_service;

    std::list<ChatServer> servers;
    for (int i = 1; i < argc; ++i)
    {
      tcp::endpoint endpoint(tcp::v4(), std::atoi(argv[i]));
      servers.emplace_back(io_service, endpoint, argv[i]);
    }

    io_service.run();
  }
  catch (std::exception &e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}