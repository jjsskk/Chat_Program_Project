
#include "chat_server.h"
using namespace std;

ChatServer::ChatServer(boost::asio::io_service &io_service,
                       const tcp::endpoint &endpoint, string port)
    : io_service_(io_service), acceptor_(io_service, endpoint),
      socket_(io_service), port_(port)
{
  do_accept();
}

void ChatServer::do_accept()
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
