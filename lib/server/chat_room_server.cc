
#include "chat_room_server.h"

  ChatRoom::ChatRoom(int room_id, std::string room_name) : room_id_(room_id), room_name_(room_name)
  {
    std::cout << " ChatRoom id: " << room_id_ << " serviced" << std::endl;
  }
  ChatRoom::~ChatRoom()
  {
    std::cout << " ChatRoom id: " << room_id_ << " terminated" << std::endl;
  }


  std::string ChatRoom::getroomname()
  {
    return room_name_;
  }
  int ChatRoom::getroomid()
  {
    return room_id_;
  }
  void ChatRoom::leave(chat_participant_ptr participant, std::string client_id)
  {
    participants_.erase(participant);
    auto it = clientid_list_.begin();
    for (; it != clientid_list_.end(); it++)
      if (client_id == (*it))
      {
        it = clientid_list_.erase(it);
        std::cout << client_id << " leave " << room_name_ << "_" << room_id_ << std::endl;
      }
    printf("here2\n");
  }

  void ChatRoom::join_user(std::string client_id, std::shared_ptr<ChatParticipant> participant)
  {

    clientid_list_.push_back(client_id);
    participants_.insert(participant); // new client

    std::cout << client_id << " join " << room_name_ << "_" << room_id_ << std::endl;
    if (!recent_msgs_.empty())
      for (auto msg : recent_msgs_) // 채팅방 안 모든 유저에게 최근 주고받은 msg 전송
        participant->deliver(msg);  // join()이 끝날떄까지는 read msg 전송 못함
  }

  void ChatRoom::deliver(struct packet &msg)
  {
    printf("room of deliver executed\n");
    recent_msgs_.push_back(msg);
    while (recent_msgs_.size() > max_recent_msgs)
      recent_msgs_.pop_front();

    for (auto participant : participants_) // 채팅방 안 모든 유저에게 read받은 msg 전송
      participant->deliver(msg);
  }
  void ChatRoom::deliver(struct packet &msg, std::shared_ptr<ChatParticipant> myself)
  {
    // printf("room of deliver executed\n");
    // recent_msgs_.push_back(msg);
    // while (recent_msgs_.size() > max_recent_msgs)
    //   recent_msgs_.pop_front();
    int i = 0;
    for (auto participant : participants_) // server send file read check msg to all member in room except himself
    {
      if (participant != myself)
      {
        // printf("why twice\n");
        participant->deliver(msg);
      }
      i++;
    }
    // printf("particpants number : %d\n",i);
  }
  std::string ChatRoom::get_all_client_id(std::string client_id)
  {
    namelist_.clear();
    namelist_ = "";
    int num = 0;
    for (auto id : clientid_list_)
    {
      if (!(id == client_id))
      {
        // std::cout<<client_id<<std::endl;
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

