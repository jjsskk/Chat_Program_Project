#ifndef CHAT_SEESION_H
#define CHAT_SEESION_H
#include <boost/asio.hpp>
#include <cstdlib>

#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include "chat_participant.h"
#include "chat_room_server.h"
#include "globalVariable.h"
#include "struct_pkt.h"

using boost::asio::ip::tcp;
using namespace std;

class ChatSession
    : public enable_shared_from_this<ChatSession>,
      public ChatParticipant
{
public:
  ChatSession(boost::asio::io_service &io_service, tcp::socket socket, list<shared_ptr<ChatRoom>> &roomlist, string port);
  ~ChatSession();
  void Start();
  void MakeRoomList();
  void Leave();
  struct packet *GetPacket();
  void Deliver(struct packet &msg);

private:
  void DoWrite();

  void FileUpload(int thread_port);
  void DoRead(); 

  void DoCreateRoom();
  void NotifyCreatedRoom();
  void DoEnterRoom();

  boost::asio::io_service &io_service_;
  tcp::socket socket_;
  string port_;
  list<shared_ptr<ChatRoom>> &roomlist_;
  deque<struct packet> write_msgs_; // store msgs to bo going to send to corresponing client in time-order
  string namelist_;
  string idlist_;
  string client_id_;
  struct packet *pkt_ = (struct packet *)malloc(sizeof(struct packet));
  shared_ptr<ChatRoom> current_room_; // room that user enters in currently
  char file_name_[20];
  FILE *fp = NULL;
  vector<thread> threadpool;

};

#endif