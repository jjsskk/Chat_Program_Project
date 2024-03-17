#include "chat_session_server.h"

void ChatSession::DoRead()
{
  auto self(shared_from_this());
  boost::asio::async_read(socket_,
                          boost::asio::buffer(pkt_, sizeof(struct packet)),             // header length만큼만 읽겠다
                          [this, self](boost::system::error_code ec, size_t /*length*/) // 나머지 body length는 buffer에 남음-
                          {
                            if (!ec)
                            {
                              if (pkt_->type == 0) // sever read created roomname and client id
                              {
                                cout << " read selectd room name :" << pkt_->selected_roomname << endl;
                                client_id_.clear();
                                client_id_.append(pkt_->client_id);
                                // client_id_+=c_str(pkt_->client_id);
                                cout << client_id_ << " connected" << endl;
                                DoCreateRoom();
                                NotifyCreatedRoom();
                                DoRead();
                                return;
                              }
                              if (pkt_->type == 1) // server read selected roomname(already existed) and client_id
                              {
                                cout << " read selectd room name :" << pkt_->selected_roomname << endl;
                                client_id_.clear();
                                client_id_.append(pkt_->client_id);
                                cout << client_id_ << " connected" << endl;
                                DoEnterRoom();
                                DoRead();
                              }
                              if (pkt_->type == 2) // server read msg from client
                              {
                                cout << " msg :" << pkt_->msg << endl;
                                pkt_->type = 3;
                                current_room_->Deliver(*pkt_); // server send msg to all member in room
                                DoRead();
                                return;
                              }
                              if (pkt_->type == 3) // server read file information and new port number from client
                              {

                                memset(file_name_, 0, sizeof(file_name_));
                                strcpy(file_name_, pkt_->file_name);
                                threadpool.emplace_back(&ChatSession::FileUpload, this, pkt_->port);
                                threadpool.back().detach();
                                // thread t(&ChatSession::FileUpload, this);
                                // t.detach();

                                DoRead();
                                return;
                              }
                              if (pkt_->type == 4) // read file information and notification that file upload is done from client
                              {
                                // cout<<" msg :" << pkt_->msg<<endl;

                                pkt_->type = 4;
                                memset(file_name_, 0, sizeof(file_name_));
                                strcpy(file_name_, pkt_->file_name);
                                current_room_->Deliver(*pkt_, shared_from_this());
                                // server send a question asking if you want to download this file
                                // to all participants in this room except file owner

                                DoRead();
                                return;
                              }
                              if (pkt_->type == 5) // server read answer about whether client want to receive file
                              {
                                // cout<<" msg :" << pkt_->msg<<endl;
                                memset(file_name_, 0, sizeof(file_name_));
                                strcpy(file_name_, pkt_->file_name);
                                if (pkt_->file_transfer_check == 1) // server send file to client saying 'yes'
                                {
                                  cout << file_name_ << endl;
                                  // threadpool.emplace_back(&ChatSession::file_download,this);
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
                                      Deliver(*pkt_);
                                      break;
                                    }
                                    Deliver(*pkt_);
                                  }
                                  fclose(fp);
                                }

                                DoRead();
                                return;
                              }
                              if (pkt_->type == 6) // read news that one client left the room.
                              {
                                current_room_->Leave(shared_from_this(), client_id_);
                                pkt_->type = 6;
                                memset(pkt_->client_id, 0, NAME_SIZE);
                                strcpy(pkt_->client_id, client_id_.c_str());
                                current_room_->Deliver(*pkt_); // server send msg to all member in room except leaved client
                                current_room_ = nullptr;
                                printf("here\n");
                                // Leave();
                                Start();
                                return;
                              }
                              if (pkt_->type == 7) // read news that one client terminate chat program
                              {
                                if (current_room_ != nullptr) // for client  unexpected close
                                {
                                  current_room_->Leave(shared_from_this(), client_id_);
                                  pkt_->type = 6;
                                  memset(pkt_->client_id, 0, NAME_SIZE);
                                  strcpy(pkt_->client_id, client_id_.c_str());
                                  current_room_->Deliver(*pkt_); // server send msg to all member in room except leaved client
                                }
                                participants_life_.erase(shared_from_this()); // to manage client_session object life cycle
                                return;
                              }
                            }
                            else
                            {
                              cerr << "Exception: " << ec.message() << "\n";
                              if (current_room_ != nullptr) // for client  unexpected close
                              {
                                current_room_->Leave(shared_from_this(), client_id_);
                                pkt_->type = 6;
                                memset(pkt_->client_id, 0, NAME_SIZE);
                                strcpy(pkt_->client_id, client_id_.c_str());
                                current_room_->Deliver(*pkt_); // server send msg to all member in room except leaved client
                              }
                              participants_life_.erase(shared_from_this());
                              // room_.Leave(shared_from_this());
                            }
                          });
}

void ChatSession::DoWrite()
{
  // sleep(5);
  auto self(shared_from_this());
  boost::asio::async_write(socket_,
                           boost::asio::buffer(&(write_msgs_.front()), sizeof(struct packet)),
                           [this, self](boost::system::error_code ec, size_t /*length*/)
                           {
                             if (!ec)
                             {
                               write_msgs_.pop_front();
                               if (!write_msgs_.empty())
                               {
                                 DoWrite();
                               }
                             }
                             else
                             {

                               cerr << "Exception: " << ec.message() << "\n";
                               if (current_room_ != nullptr)
                               {
                                 current_room_->Leave(shared_from_this(), client_id_);
                                 pkt_->type = 6;
                                 memset(pkt_->client_id, 0, NAME_SIZE);
                                 strcpy(pkt_->client_id, client_id_.c_str());
                                 current_room_->Deliver(*pkt_);
                               }
                               participants_life_.erase(shared_from_this());
                             }
                           });
}