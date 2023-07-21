class chat_participant
{
public:
  virtual ~chat_participant() {}
  virtual void deliver(struct packet &msg) = 0;
  virtual void make_roomlist() = 0;
  virtual struct packet* getPacket() = 0;
  // virtual std::string get_client_id() =0;
};

typedef std::shared_ptr<chat_participant> chat_participant_ptr;
class chat_room
{
public:
  chat_room(int room_id, std::string room_name) : room_id_(room_id), room_name_(room_name)
  {
    std::cout << " chat_room id: " << room_id_ << " serviced" << std::endl;
  }
  ~chat_room()
  {
    std::cout << " chat_room id: " << room_id_ << " terminated" << std::endl;
  }

public:
  std::string getroomname()
  {
    return room_name_;
  }
  int getroomid()
  {
    return room_id_;
  }
  void leave(chat_participant_ptr participant, std::string client_id)
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

  void join_user(std::string client_id, std::shared_ptr<chat_participant> participant)
  {

    clientid_list_.push_back(client_id);
    participants_.insert(participant); // new client

    std::cout << client_id << " join " << room_name_ << "_" << room_id_ << std::endl;
    if (!recent_msgs_.empty())
      for (auto msg : recent_msgs_) // 채팅방 안 모든 유저에게 최근 주고받은 msg 전송
        participant->deliver(msg);  // join()이 끝날떄까지는 read msg 전송 못함
  }

  void deliver(struct packet &msg)
  {
    printf("room of deliver executed\n");
    recent_msgs_.push_back(msg);
    while (recent_msgs_.size() > max_recent_msgs)
      recent_msgs_.pop_front();

    for (auto participant : participants_) // 채팅방 안 모든 유저에게 read받은 msg 전송
      participant->deliver(msg);
  }
  void deliver(struct packet &msg, std::shared_ptr<chat_participant> myself)
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
  std::string get_all_client_id(std::string client_id)
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
  std::string room_name_;
  int room_id_;
  std::string namelist_;
};
