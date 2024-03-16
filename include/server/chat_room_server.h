#ifndef CHAT_ROOM_H
#define CHAT_ROOM_H

#include "chat_participant.h"
#include <iostream>
#include <deque>
#include <list>
#include <set>
#include "globalVariable.h"
#include "struct_pkt.h"

using namespace std;

class ChatRoom
{
public:
  ChatRoom(int room_id, string room_name);
  ~ChatRoom();

public:
  string GetRoomName();
  int GetRoomId();
  void Leave(chat_participant_ptr participant, string client_id);
  void JoinUser(string client_id, shared_ptr<ChatParticipant> participant);

  void Deliver(struct packet &msg);
  void Deliver(struct packet &msg, shared_ptr<ChatParticipant> myself);
  string GetAllClientId(string client_id);

private: 
  list<string> clientid_list_;
  set<chat_participant_ptr> participants_;      // client_id
  enum
  {
    max_recent_msgs = 100
  };
  deque<struct packet> recent_msgs_; // 방안에서 최근 read받은 msg 저장
  int room_id_;
  string room_name_;
  string namelist_;
};

#endif