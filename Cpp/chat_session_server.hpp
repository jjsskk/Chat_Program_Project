class chat_session
    : public std::enable_shared_from_this<chat_session>,
      public chat_participant
{
public:
  chat_session(boost::asio::io_service &io_service, tcp::socket socket, std::list<std::shared_ptr<chat_room>> &roomlist, std::string port)
      : io_service_(io_service), socket_(std::move(socket)), port_(port),
        roomlist_(roomlist)
  {
    // start();
  }
  ~chat_session()
  {
    free(pkt_);
    socket_.close();
    std::cout << "chat_session terminated" << std::endl;
  }
  void start()
  {

    make_roomlist();
    deliver(*pkt_);
    do_read();
    // do_read_room();
    //  std::make_shared<chat_session>(std::move(socket_), room_)->start();
  }
  void make_roomlist()
  {
    namelist_.clear();
    idlist_.clear();
    namelist_ = "";
    idlist_ = "";
    for (auto room : roomlist_)
    {

      namelist_ += room->getroomname();
      namelist_ += "/";
      idlist_ += std::to_string(room->getroomid());
      idlist_ += "/";
    }
    if (roomlist_.empty())
    {
      namelist_ = "There is no room!!";
      idlist_ = "no id_list";
    }
    memset(pkt_->all_name, 0, BUF_SIZE);
    memset(pkt_->all_room_id, 0, BUF_SIZE);
    strcpy(pkt_->all_name, namelist_.c_str());
    strcpy(pkt_->all_room_id, idlist_.c_str());
    pkt_->type = 0; // send room list to corrsponding client

  }
  void leave()
  {
    // to_do remove chat_room in roomlist

    io_service_.post(
        [this]()
        {
          printf("restart\n");
          start();
        });
  }
  struct packet* getPacket()
  {
    return pkt_;
  }

  void deliver(struct packet &msg) // 채팅방 안 모든 유저에게 read받은 msg 전송
  {
    // fflush(stdout);
    printf("parcipant deliver :%s\n", msg.all_name);
    //  printf("hell3\n");
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.push_back(msg); // 전송할 msg(read받은 msg) 저장
    std::cout << write_in_progress << std::endl;
    if (!write_in_progress) // write_in progress =true(do_write()진행중) = write_msgs가 비어있지 않음
    {
      // printf("hell2\n");
      do_write();
    }
    // main thread 1개 기준으로 코드 짜짐
    // read가 msg가 들어 올때 마다 위의 deliver()가 호출된다. 만약 do_write() 진행을
    //  다 마지치 못한 상황에서 비동기 read의 handler 호출로 현재 deliver()가 호출되면
    // do_write()진행을 못마치고 온 상태라서 (write_in progress =true) do_write()를 다시 호출 못하게 막는다 (대신에 write_msgs_에 못보낸 read msg를 차곡차곡 쌓음)
    // 그후 아까 중단됬던 do_Write를 다시 진행해서 write_msgs큐(그동안 못보낸 read msg가  큐에계속 쌓임)를 모두 보내서 모두 비우면 다시 read했을때 do_write()한다
  }



private:
  void do_write()
  {
    // sleep(5);
    auto self(shared_from_this());
    boost::asio::async_write(socket_,
                             boost::asio::buffer(&(write_msgs_.front()), sizeof(struct packet)),
                             [this, self](boost::system::error_code ec, std::size_t /*length*/)
                             {
                               if (!ec)
                               {
                                 // printf("hell\n");
                                 write_msgs_.pop_front();
                                 if (!write_msgs_.empty())
                                 {
                                   do_write();
                                 }
                               }
                               else
                               {

                                 std::cerr << "Exception: " << ec.what() << "\n";
                                 if (current_room_ != nullptr)
                                 {
                                   current_room_->leave(shared_from_this(), client_id_);
                                   pkt_->type = 6;
                                   memset(pkt_->client_id, 0, NAME_SIZE);
                                   strcpy(pkt_->client_id, client_id_.c_str());
                                   current_room_->deliver(*pkt_);
                                 }
                                 participants_life_.erase(shared_from_this());
                                 //  room_.leave(shared_from_this());
                               }
                             });
  }

  void file_upload(int thread_port)
  {
    boost::asio::io_service io_service;
    tcp::endpoint endpoint(tcp::v4(), thread_port + 10000);
    tcp::acceptor acceptor(io_service, endpoint);

    tcp::socket socket(io_service);
    struct packet pkt_file;
    try
    {

      fp = fopen(file_name_, "wb");
      if (fp == NULL)
        printf("fp error\n");
      acceptor.accept(socket);
      printf("here : %d\n", thread_port);
      // while((read_cnt=read(clnt_sd,buf,BUF_SIZE))!=0)
      int read_length = 0;
      int file_size = 0;
      socket.read_some(boost::asio::buffer(&pkt_file, sizeof(struct packet)));
      int file_size_client = pkt_file.file_size;

      char file[FILE_SIZE];
      printf("total file size :%d\n", file_size_client);
      while ((read_length = socket.read_some(boost::asio::buffer(file, FILE_SIZE))) != 0)
        fwrite((void *)(file), 1, read_length, fp);

      //   {
      //     file_size += read_length;
      // //  printf("readread!!%d\n",pkt_file.end_read_size);
      //     if(file_size == file_size_client )
      //     {
      //       fwrite((void*)(pkt_file.file_content),1,pkt_file.end_read_size,fp);
      //         break;
      //     }

      //   fwrite((void*)(pkt_file.file_content),1,FILE_SIZE,fp);
      //   }
      printf("writewrite!!\n");

      socket.write_some(boost::asio::buffer(file, read_length));
    }
    catch (std::exception &e)
    {
      std::cerr << "Exception: " << e.what() << "\n";
      socket.close();
    }

    printf(" file upload done\n");
    fclose(fp);
    fp = NULL;
    socket.close();
  }
  // void file_download()
  // {

  // }

  void do_read(); // type = 0->read created room name and client_id
                  //  or type =1 ->read existed room name and room_id and client_id

  void do_create_room();
  void notify_created_room();
  void do_enter_room();

  boost::asio::io_service &io_service_;
  std::deque<struct packet> write_msgs_; // store msgs to bo going to send to corresponing client in time-order
  std::string namelist_;
  std::string idlist_;
  std::string client_id_;
  struct packet *pkt_ = (struct packet *)malloc(sizeof(struct packet));
  tcp::socket socket_;
  std::shared_ptr<chat_room> current_room_; // room that user enters in currently
  std::list<std::shared_ptr<chat_room>> &roomlist_;
  char file_name_[20];
  FILE *fp = NULL;
  std::string port_;
  std::vector<std::thread> threadpool;
  // std::shared_ptr<chat_session> session_=shared_from_this();
  // 서버가 돌아가는 동안에는 계속 살아있음!! 즉 누군가 방을 만든 시점부터
  // 내용이 msg 저장됨 ->즉 접속한 새로운 유저는 이방 정보(객체)를 계속 가져감 접속할떄마다 새로 방 만들어서 가는게 x
};