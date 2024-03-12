#ifndef CHAT_ROOM_H
#define CHAT_ROOM_H

#include "chat_participant.h"
#include <iostream>
#include <deque>
#include <list>
#include <set>
#include "globalVariable.h"
#include "struct_pkt.h"

class ChatRoom
{
public:
  ChatRoom(int room_id, std::string room_name);
  ~ChatRoom();

public:
  std::string GetRoomName();
  int GetRoomId();
  void Leave(chat_participant_ptr participant, std::string client_id);
  void JoinUser(std::string client_id, std::shared_ptr<ChatParticipant> participant);

  void Deliver(struct packet &msg);
  void Deliver(struct packet &msg, std::shared_ptr<ChatParticipant> myself);
  std::string GetAllClientId(std::string client_id);

private: 
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