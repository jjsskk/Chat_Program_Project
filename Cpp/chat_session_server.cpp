void chat_session::do_read() // type = 0->read created room name and client_id
                             //  or type =1 ->read existed room name and room_id and client_id
{
  auto self(shared_from_this());
  boost::asio::async_read(socket_,
                          boost::asio::buffer(pkt_, sizeof(struct packet)),                  // header length만큼만 읽겠다
                          [this, self](boost::system::error_code ec, std::size_t /*length*/) // 나머지 body length는 buffer에 남음-
                          {
                            if (!ec)
                            {
                              if (pkt_->type == 0) // sever read created roomname and client id
                              {
                                std::cout << " read selectd room name :" << pkt_->selected_roomname << std::endl;
                                client_id_.clear();
                                client_id_.append(pkt_->client_id);
                                // client_id_+=std::c_str(pkt_->client_id);
                                std::cout << client_id_ << " connected" << std::endl;
                                do_create_room();
                                notify_created_room();
                                do_read();
                              }
                              else if (pkt_->type == 1) // server read selected roomname(already existed) and client_id
                              {
                                std::cout << " read selectd room name :" << pkt_->selected_roomname << std::endl;
                                client_id_.clear();
                                client_id_.append(pkt_->client_id);
                                std::cout << client_id_ << " connected" << std::endl;
                                do_enter_room();
                                do_read();
                              }
                              else if (pkt_->type == 2) // server read msg from client
                              {
                                std::cout << " msg :" << pkt_->msg << std::endl;
                                pkt_->type = 3;
                                current_room_->deliver(*pkt_); // server send msg to all member in room
                                do_read();
                              }
                              else if (pkt_->type == 3) // server read upload file from client
                              {

                                memset(file_name_, 0, sizeof(file_name_));
                                strcpy(file_name_, pkt_->file_name);
                                threadpool.emplace_back(&chat_session::file_upload, this, pkt_->port);
                                threadpool.back().detach();
                                // std::thread t(&chat_session::file_upload, this);
                                // t.detach();

                                // if(fp == NULL)
                                // 	fp = fopen(file_name_, "wb");
                                // // while((read_cnt=read(clnt_sd,buf,BUF_SIZE))!=0)
                                // if(pkt_->end_read_size == -1 )
                                // fwrite((void*)(pkt_->file_content),1,FILE_SIZE,fp);
                                // else
                                // {
                                //   printf("end : %d\n",pkt_->end_read_size);
                                // fwrite((void*)(pkt_->file_content),1,pkt_->end_read_size,fp);
                                // fclose(fp);
                                // fp = NULL;
                                // }
                                do_read();
                              }
                              else if (pkt_->type == 4) // server read file request from client
                              {
                                // std::cout<<" msg :" << pkt_->msg<<std::endl;

                                pkt_->type = 4;
                                memset(file_name_, 0, sizeof(file_name_));
                                strcpy(file_name_, pkt_->file_name);
                                current_room_->deliver(*pkt_, shared_from_this()); // server send file read check msg to all member in room except himself
                                // std::thread t(&chat_session::file_transfer, this,(pkt_->port)+10000);
                                //   t.detach();

                                do_read();
                              }
                              else if (pkt_->type == 5) // server read ack about whether client want to receive file
                              {
                                // std::cout<<" msg :" << pkt_->msg<<std::endl;
                                memset(file_name_, 0, sizeof(file_name_));
                                strcpy(file_name_, pkt_->file_name);
                                if (pkt_->file_transfer_check == 1) // server send file to client saying 'yes'
                                {
                                  std::cout << file_name_ << std::endl;
                                  // threadpool.emplace_back(&chat_session::file_download,this);
                                  // threadpool.back().detach();
                                  fp = fopen(file_name_, "rb");
                                  if (fp == NULL)
                                    printf("fp error null\n");
                                  int file_size = 0;
                                  int read_cnt = 0;
                                  while (1)
                                  {
                                    pkt_->end_read_size = -1;
                                    pkt_->type = 5;
                                    read_cnt = fread((void *)(pkt_->file_content), 1, FILE_SIZE, fp);
                                    file_size += read_cnt;
                                    // if(file_size%1024==0)
                                    // printf("send %d bytes \n",read_size);
                                    if (read_cnt < FILE_SIZE)
                                    {
                                      pkt_->type = 5;
                                      pkt_->end_read_size = read_cnt;
                                      deliver(*pkt_);
                                      break;
                                    }
                                    deliver(*pkt_);
                                    // socket_.write_some(boost::asio::buffer(pkt_, sizeof(struct packet)));
                                  }
                                  fclose(fp);
                                }

                                do_read();
                              }

                              else if (pkt_->type == 6)
                              {
                                current_room_->leave(shared_from_this(), client_id_);
                                pkt_->type = 6;
                                memset(pkt_->client_id, 0, NAME_SIZE);
                                strcpy(pkt_->client_id, client_id_.c_str());
                                current_room_->deliver(*pkt_); // server send msg to all member in room except leaved client
                                current_room_ = nullptr;
                                printf("here\n");
                                // leave();
                                start();
                              }
                              else if (pkt_->type == 7)
                              {
                                // printf("here\n");
                                if (current_room_ != nullptr) // for client  unexpected close
                                {
                                  current_room_->leave(shared_from_this(), client_id_);
                                  pkt_->type = 6;
                                  memset(pkt_->client_id, 0, NAME_SIZE);
                                  strcpy(pkt_->client_id, client_id_.c_str());
                                  current_room_->deliver(*pkt_); // server send msg to all member in room except leaved client
                                }
                                participants_life_.erase(shared_from_this()); // to manage client_session object life cycle
                              }
                            }
                            else
                            {
                              std::cerr << "Exception: " << ec.what() << "\n";
                              if (current_room_ != nullptr) // for client  unexpected close
                              {
                                current_room_->leave(shared_from_this(), client_id_);
                                pkt_->type = 6;
                                memset(pkt_->client_id, 0, NAME_SIZE);
                                strcpy(pkt_->client_id, client_id_.c_str());
                                current_room_->deliver(*pkt_); // server send msg to all member in room except leaved client
                              }
                              participants_life_.erase(shared_from_this());
                              // room_.leave(shared_from_this());
                            }
                          });
}

void chat_session::do_create_room()
{
  std::string name(pkt_->selected_roomname);
  std::shared_ptr<chat_room> ptr =
      std::make_shared<chat_room>(room_id++, name);
  roomlist_.push_back(ptr);
  current_room_ = ptr;
  std::string client_id(pkt_->client_id);
  roomlist_.back()->join_user(client_id, shared_from_this()); // save client_id and socket in room
                                                              //  memset(pkt_->client_id,0,NAME_SIZE);
  pkt_->type = 2;
  // strcpy(pkt_->client_id,client_id_.c_str());
  roomlist_.back()->deliver(*pkt_); // send new client name to all member in room ->print that 000 has joined in the room of all clients cmd
  memset(pkt_->all_name, 0, BUF_SIZE);
  pkt_->type = 1;
  strcpy(pkt_->all_name, roomlist_.back()->get_all_client_id(client_id_).c_str());
  deliver(*pkt_); // send all user name to new client; ->dont save recent_msgs_ in chat room
                  //-> print that 000, welcome you in new client cmd

  // insert_newroom();
}

void chat_session::do_enter_room()
{
  auto it = roomlist_.begin();
  for (; it != roomlist_.end(); it++)
    if (pkt_->selected_room_id == (*it)->getroomid())
    {
      std::string client_id(pkt_->client_id);
      current_room_ = *it;
      (*it)->join_user(client_id, shared_from_this());
      //  memset(pkt_->client_id,0,NAME_SIZE);
      pkt_->type = 2;
      // strcpy(pkt_->client_id,client_id_.c_str());
      // roomlist_.back()->deliver(*pkt_); // send new client name to all member in room ->print that 000 has joined in the room of all clients cmd
      current_room_->deliver(*pkt_); // send new client name to all member in room ->print that 000 has joined in the room of all clients cmd
      memset(pkt_->all_name, 0, BUF_SIZE);
      pkt_->type = 1;
      strcpy(pkt_->all_name, (*it)->get_all_client_id(client_id_).c_str());
      deliver(*pkt_); // send all user name to new client; ->dont save recent_msgs_ in chat room
                      //-> print that 000, welcome you in new client cmd
      break;
    }
}

void chat_session::notify_created_room()
{
  auto it = roomlist_.begin();
  for (auto participant : participants_life_)
    {
      if(participant == shared_from_this())
      continue;
      participant->make_roomlist();
      participant->deliver(*(participant->getPacket())); 
    }
}