#ifndef CHAT_SEVER_H
#define CHAT_SEVER_H

#include "chat_session_server.h"
#include "globalVariable.h"

using namespace std;

class ChatServer
{
public:
    ChatServer(boost::asio::io_service &io_service,
               const tcp::endpoint &endpoint, string port);

private:
    void do_accept();

    boost::asio::io_service &io_service_;
    tcp::acceptor acceptor_;
    tcp::socket socket_;
    list<shared_ptr<ChatRoom>> roomlist_;
    string port_;
};

#endif