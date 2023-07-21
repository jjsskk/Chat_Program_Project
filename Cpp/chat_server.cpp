
#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <utility>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;
#define BUF_SIZE 100
#define NAME_SIZE 20
#define FILE_SIZE 500

int room_id = 0;

struct packet
{
  short type;
  char client_id[NAME_SIZE];
  char selected_roomname[NAME_SIZE];
  int selected_room_id;
  char msg[BUF_SIZE];
  char all_name[BUF_SIZE]; // used both in all room name and in all user name
  char all_room_id[BUF_SIZE];
  short file_transfer_check;
  char file_name[NAME_SIZE];
  int file_size;
  char file_content[FILE_SIZE];
  int end_read_size;
  int port;
};

#include "chat_room_server.cpp"
std::set<chat_participant_ptr> participants_life_; // to manage client_session object life cycle
#include "chat_session_server.hpp"
#include "chat_session_server.cpp"

class chat_server
{
public:
  chat_server(boost::asio::io_service &io_service,
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
                               //  participants_life_.insert(std::make_shared<chat_session>(io_service_,std::move(socket_), roomlist_));
                               std::shared_ptr<chat_session> ptr = std::make_shared<chat_session>(io_service_, std::move(socket_), roomlist_, port_);
                               ptr->start();
                               participants_life_.insert(ptr);
                             }

                             do_accept();
                           });
  }
  boost::asio::io_service &io_service_;
  tcp::acceptor acceptor_;
  tcp::socket socket_;
  std::list<std::shared_ptr<chat_room>> roomlist_;
  std::string port_;
};

//----------------------------------------------------------------------

int main(int argc, char *argv[])
{
  try
  {
    if (argc < 2)
    {
      std::cerr << "Usage: chat_server <port> [<port> ...]\n";
      return 1;
    }

    boost::asio::io_service io_service;

    std::list<chat_server> servers;
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