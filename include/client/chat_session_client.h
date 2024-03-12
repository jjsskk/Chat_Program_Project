#ifndef CHAT_SEESION_H
#define CHAT_SEESION_H

#include "globalVariable.h"
#include <cstdlib>

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <deque>
#include <iostream>
using boost::asio::ip::tcp;

class ChatSession
{
public:
  ChatSession(boost::asio::io_service &io_service,
               tcp::resolver::iterator endpoint_iterator, std::string client_id, std::string host, std::string port);
  ~ChatSession();

  void Write(struct packet &msg);

  int UserinputInRoomList(); // main thread execute this method
  void DisplayRoomList();
  void DoCommunicateInRoom();

  void Close();

  void FileUpload(int thread_port);

private:
  void DoConnect(tcp::resolver::iterator endpoint_iterator);

  void DoRead();
  void DoWrite();

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