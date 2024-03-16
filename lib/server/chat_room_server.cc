
#include "chat_room_server.h"

  ChatRoom::ChatRoom(int room_id, string room_name) : room_id_(room_id), room_name_(room_name)
  {
    cout << " ChatRoom id: " << room_id_ << " serviced" << endl;
  }
  ChatRoom::~ChatRoom()
  {
    cout << " ChatRoom id: " << room_id_ << " terminated" << endl;
  }


  string ChatRoom::GetRoomName()
  {
    return room_name_;
  }
  int ChatRoom::GetRoomId()
  {
    return room_id_;
  }
  void ChatRoom::Leave(chat_participant_ptr participant, string client_id)
  {
    participants_.erase(participant);
    auto it = clientid_list_.begin();
    for (; it != clientid_list_.end(); it++)
      if (client_id == (*it))
      {
        it = clientid_list_.erase(it);
        cout << client_id << " leave " << room_name_ << "_" << room_id_ << endl;
      }
    printf("here2\n");
  }

  void ChatRoom::JoinUser(string client_id, shared_ptr<ChatParticipant> participant)
  {

    clientid_list_.push_back(client_id);
    participants_.insert(participant); // new client

    cout << client_id << " join " << room_name_ << "_" << room_id_ << endl;
    if (!recent_msgs_.empty())
      for (auto msg : recent_msgs_) // 채팅방 안 모든 유저에게 최근 주고받은 msg 전송
        participant->Deliver(msg);  // join()이 끝날떄까지는 read msg 전송 못함
  }

  void ChatRoom::Deliver(struct packet &msg)
  {
    printf("room of deliver executed\n");
    recent_msgs_.push_back(msg);
    while (recent_msgs_.size() > max_recent_msgs)
      recent_msgs_.pop_front();

    for (auto participant : participants_) // 채팅방 안 모든 유저에게 read받은 msg 전송
      participant->Deliver(msg);
  }
  void ChatRoom::Deliver(struct packet &msg, shared_ptr<ChatParticipant> myself)
  {
 
    int i = 0;
    for (auto participant : participants_) // server send file read check msg to all member in room except himself
    {
      if (participant != myself)
      {
        // printf("why twice\n");
        participant->Deliver(msg);
      }
      i++;
    }
    // printf("particpants number : %d\n",i);
  }
  string ChatRoom::GetAllClientId(string client_id)
  {
    namelist_.clear();
    namelist_ = "";
    int num = 0;
    for (auto id : clientid_list_)
    {
      if (!(id == client_id))
      {
        // cout<<client_id<<endl;
        namelist_ += "'";
        namelist_ += id;
        namelist_ += "'";
        namelist_ += " ";
        num++;
      }
    }
    if (clientid_list_.empty() || num == 0)
    {
      namelist_ = "There is no member in this room.";
    }
    else
    {
      namelist_ += "welcome you!!";
    }
    return namelist_;
  }

