#ifndef CHAT_SEESION_H
#define CHAT_SEESION_H

#include "globalVariable.h"
#include <cstdlib>

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <deque>
#include <iostream>
using boost::asio::ip::tcp;

class chat_session
{
public:
  chat_session(boost::asio::io_service &io_service,
               tcp::resolver::iterator endpoint_iterator, std::string client_id, std::string host, std::string port);
  ~chat_session();

  void write(struct packet &msg);

  int userinput_in_roomlist(); // main thread execute this method
  void display_roomlist();
  void do_communicate_in_room();

  void close();

  void file_upload(int thread_port);

private:
  void do_connect(tcp::resolver::iterator endpoint_iterator);

  void do_read();
  void do_write();

private:
  boost::asio::io_service &io_service_;
  tcp::socket socket_;
  std::string client_id_;
  std::string host_;
  std::string port_;
  std::string selectd_room_;
  std::vector<std::string> roomlist_;
  std::vector<std::string> roomidlist_;
  // chat_message read_msg_;
  struct packet *pkt_read = (struct packet *)malloc(sizeof(struct packet));
  std::deque<struct packet> write_msgs_;
  FILE *fp = NULL;
  char file_name_[20];
};

#endif