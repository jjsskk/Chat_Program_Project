#ifndef CHAT_ROOM_H
#define CHAT_ROOM_H

#include "chat_participant.h"
#include <iostream>
#include <deque>
#include <list>
#include <set>
#include "globalVariable.h"
#include "struct_pkt.h"

class chat_room
{
public:
  chat_room(int room_id, std::string room_name);
  ~chat_room();

public:
  std::string getroomname();
  int getroomid();
  void leave(chat_participant_ptr participant, std::string client_id);
  void join_user(std::string client_id, std::shared_ptr<chat_participant> participant);

  void deliver(struct packet &msg);
  void deliver(struct packet &msg, std::shared_ptr<chat_participant> myself);
  std::string get_all_client_id(std::string client_id);

  // do_write_
private: // 전역 변수들
  std::list<std::string> clientid_list_;
  std::set<chat_participant_ptr> participants_;      // client_id
  std::set<chat_participant_ptr> participants_file_; // client_id
  enum
  {
    max_recent_msgs = 100
  };
  std::deque<struct packet> recent_msgs_; // 방안에서 최근 read받은 msg 저장
  int room_id_;
  std::string room_name_;
  std::string namelist_;
};

#endif