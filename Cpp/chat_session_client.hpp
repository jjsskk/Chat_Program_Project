class chat_session
{
public:
  chat_session(boost::asio::io_service &io_service,
               tcp::resolver::iterator endpoint_iterator, std::string client_id, std::string host, std::string port)
      : io_service_(io_service),
        socket_(io_service), client_id_(client_id), host_(host), port_(port)
  {
    do_connect(endpoint_iterator);
  }
  ~chat_session()
  {
    free(pkt_read);
    socket_.close();
  }

  void write(struct packet &msg);

  int userinput_in_roomlist(); // main thread execute this method
  void display_roomlist();
  void do_communicate_in_room();

  void close()
  {
    io_service_.post([this]()
                     { socket_.close(); });
  }
  void file_upload(int thread_port)
  {

    struct packet *pkt_file = (struct packet *)malloc(sizeof(struct packet));
    ;
    int read_cnt = 0;
    int file_size = 0;

    while (1)
    {
      read_cnt = fread((void *)(pkt_file->file_content), 1, FILE_SIZE, fp);
      file_size += read_cnt;
      if (read_cnt < FILE_SIZE)
      {
        memset(pkt_file->file_content, 0, FILE_SIZE);
        pkt_file->file_size = file_size;
        break;
      }
    }
    fclose(fp);
    fp = fopen(file_name_, "rb");
    sleep(2);
    boost::asio::io_service ioservice;
    tcp::socket socket(ioservice);
    tcp::resolver resolver(ioservice);
    tcp::resolver::query q{host_, std::to_string(thread_port + 10000)};
    try
    {

      connect(socket, resolver.resolve(q));
      printf("port:%d", thread_port);

      socket.write_some(boost::asio::buffer(pkt_file, sizeof(struct packet)));
      char file[FILE_SIZE]; // file transfer in struct cause data loss so i use char array for only file data
      file_size = 0;
      while (1)
      {
        pkt_file->end_read_size = -1;
        pkt_file->type = 3;
        read_cnt = fread((void *)(file), 1, FILE_SIZE, fp);
        file_size += read_cnt;
        // if(file_size%1024==0)
        // printf("send %d bytes \n",read_size);
        if (read_cnt < FILE_SIZE)
        {
          pkt_file->type = 3;
          pkt_file->end_read_size = read_cnt;
          socket.write_some(boost::asio::buffer(file, read_cnt));
          break;
        }
        socket.write_some(boost::asio::buffer(file, FILE_SIZE));
      }
      socket.shutdown(tcp::socket::shutdown_send);

      socket.read_some(boost::asio::buffer(file, read_cnt));
      printf("readread!!\n");
    }
    catch (std::exception &e)
    {
      std::cerr << "upload shutdown:  " << e.what() << "\n";
      socket.close();
    }
    socket.close();
    fclose(fp);
    fp = NULL;
    std::cout << "file upload done" << std::endl;

    memset(pkt_file->file_name, 0, NAME_SIZE);
    memset(pkt_file->client_id, 0, NAME_SIZE);
    strcpy(pkt_file->file_name, file_name_);
    strcpy(pkt_file->client_id, client_id_.c_str());
    pkt_file->type = 4;

    write(*pkt_file);
    free(pkt_file);
    pkt_file = NULL;
  }

  // void file_download()
  // {
  //                   if(fp == NULL)
  //                 fp = fopen("lake.jpg", "wb");
  //                   if(fp == NULL)
  //                     printf("fp error\n");

  //                     if(pkt_read->end_read_size != -1 )
  //                    {
  //                       fwrite((void*)(pkt_read->file_content),1,pkt_read->end_read_size,fp);
  //                       fclose(fp);
  //                       fp=NULL;
  //                    }else{
  //                     fwrite((void*)(pkt_read->file_content),1,FILE_SIZE,fp);

  //                    }
  // }

private:
  void do_connect(tcp::resolver::iterator endpoint_iterator)
  {
    boost::asio::async_connect(socket_, endpoint_iterator,
                               [this](boost::system::error_code ec, tcp::resolver::iterator)
                               {
                                 if (!ec)
                                 {
                                   do_read();
                                 }
                               });
  }

  void do_read()
  { //// be careful that you use async_read!! -> this method only read buffer when buffer filled by write data .if otherwise, don't read
    boost::asio::async_read(socket_,
                            boost::asio::buffer(pkt_read, sizeof(struct packet)),
                            [this](boost::system::error_code ec, std::size_t /*length*/)
                            {
                              if (!ec)
                              {
                                if (pkt_read->type == 0) // client read all existed room name and room id
                                {

                                  if(room == 1 )
                                  {
                                    memset(pkt_read,0,sizeof(pkt_read));
                                    do_read();
                                  }
                                  else
                                  {
                                    if( list == 1)
                                      display_roomlist();
                                    mtx_list.lock();
                                    list = 1;
                                    mtx_list.unlock();
                                    do_read();
                                  }
                                }
                                else if (pkt_read->type == 1) // client enter room and read all username
                                {

                                  // std::cout.write(read_msg_.body(), read_msg_.body_length());
                                  // std::cout << "Welcome to " << selectd_room_ << " room!!!" << std::endl;
                                  std::cout << pkt_read->all_name << std::endl;
                                  mtx_room.lock();
                                  room = 1; 
                                  mtx_room.unlock();
                                  do_read();
                                }
                                else if (pkt_read->type == 2) // clint enter room and read new client name
                                {
                                  // std::cout << "Welcome to " << selectd_room_ << " room!!!" << std::endl;
                                  std::cout << pkt_read->client_id << " has joined" << std::endl;
                                  // mtx_room.lock();
                                  // room = 1;
                                  // mtx_room.unlock();
                                  do_read();
                                }
                                else if (pkt_read->type == 3) // client read msg from server
                                {
                                  // if(strcmp(pkt_read->client_id,client_id_.c_str()))
                                  std::cout << "[" << pkt_read->client_id << "] : " << pkt_read->msg << std::endl;
                                  do_read();
                                }
                                else if (pkt_read->type == 4) // client read msg about whether he want to receive file from server
                                {
                                  file = 1; // do_communicate_room() method use this varibale to controll input from client
                                  memset(file_name_, 0, sizeof(file_name_));
                                  strcpy(file_name_, pkt_read->file_name);
                                  printf("!!!'%s' want to transfer file '%s' to you.\n!!!If you want to receive this file, please input #y(#Y) or otherwise, input #n(#N)\n", pkt_read->client_id, pkt_read->file_name);
                                  // pkt_read->type = 4;
                                  do_read();
                                }
                                else if (pkt_read->type == 5) // client said 'yes' read file from sever
                                {
                                  if (fp == NULL)
                                    fp = fopen(file_name_, "wb");
                                  if (fp == NULL)
                                    printf("file descriptor error\n");

                                  if (pkt_read->end_read_size != -1)
                                  {
                                    fwrite((void *)(pkt_read->file_content), 1, pkt_read->end_read_size, fp);
                                    fclose(fp);
                                    fp = NULL;
                                  }
                                  else
                                  {
                                    fwrite((void *)(pkt_read->file_content), 1, FILE_SIZE, fp);
                                  }

                                  // pkt_read->type = 4;
                                  do_read();
                                }
                                else if (pkt_read->type == 6) // cliet read client info leaved this room
                                {
                                  // if(strcmp(pkt_read->client_id,client_id_.c_str()))
                                  std::cout << pkt_read->client_id << " has left this room." << std::endl;
                                  do_read();
                                }
                              }
                              else
                              {
                                std::cerr << "Exception: " << ec.what() << "\n";

                                socket_.close();
                                exit(EXIT_FAILURE);
                              }
                            });
  }

  void do_write()
  {
    boost::asio::async_write(socket_,
                             boost::asio::buffer(&(write_msgs_.front()), sizeof(struct packet)),
                             [this](boost::system::error_code ec, std::size_t /*length*/)
                             {
                               if (!ec)
                               {
                                 write_msgs_.pop_front();
                                 if (!write_msgs_.empty())
                                 {
                                   do_write();
                                 }
                               }
                               else
                               {
                                 std::cerr << "Exception: " << ec.what() << "\n";

                                 socket_.close();
                               }
                             });
  }

private:
  std::string selectd_room_;
  std::vector<std::string> roomlist_;
  std::vector<std::string> roomidlist_;
  boost::asio::io_service &io_service_;
  std::string client_id_;
  tcp::socket socket_;
  // chat_message read_msg_;
  struct packet *pkt_read = (struct packet *)malloc(sizeof(struct packet));
  std::deque<struct packet> write_msgs_;
  FILE *fp = NULL;
  char file_name_[20];
  std::string host_;
  std::string port_;
};