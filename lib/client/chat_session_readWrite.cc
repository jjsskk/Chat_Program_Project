#include "chat_session_client.h"

void ChatSession::DoConnect(tcp::resolver::iterator endpoint_iterator)
{
    boost::asio::async_connect(socket_, endpoint_iterator,
                               [this](boost::system::error_code ec, tcp::resolver::iterator)
                               {
                                   if (!ec)
                                   {
                                       DoRead();
                                   }
                               });
}

void ChatSession::DoRead()
{ //// be careful that you use async_read!! -> this method only read buffer when buffer filled by write data .if otherwise, don't read
    boost::asio::async_read(socket_,
                            boost::asio::buffer(pkt_read, sizeof(struct packet)),
                            [this](boost::system::error_code ec, std::size_t /*length*/)
                            {
                                if (!ec)
                                {
                                    if (pkt_read->type == 0) // client read all existed room name and room id
                                    {

                                        if (room == 1)
                                        {
                                            memset(pkt_read, 0, sizeof(struct packet));
                                            DoRead();
                                        }
                                        else
                                        {
                                            if (list == 1)
                                                DisplayRoomList();
                                            mtx_list.lock();
                                            list = 1;
                                            mtx_list.unlock();
                                            DoRead();
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
                                        DoRead();
                                    }
                                    else if (pkt_read->type == 2) // clint enter room and read new client name
                                    {
                                        // std::cout << "Welcome to " << selectd_room_ << " room!!!" << std::endl;
                                        std::cout << pkt_read->client_id << " has joined" << std::endl;
                                        // mtx_room.lock();
                                        // room = 1;
                                        // mtx_room.unlock();
                                        DoRead();
                                    }
                                    else if (pkt_read->type == 3) // client read msg from server
                                    {
                                        // if(strcmp(pkt_read->client_id,client_id_.c_str()))
                                        std::cout << "[" << pkt_read->client_id << "] : " << pkt_read->msg << std::endl;
                                        DoRead();
                                    }
                                    else if (pkt_read->type == 4) // client read msg about whether he want to receive file from server
                                    {
                                        file = 1; // do_communicate_room() method use this varibale to controll input from client
                                        memset(file_name_, 0, sizeof(file_name_));
                                        strcpy(file_name_, pkt_read->file_name);
                                        printf("!!!'%s' want to transfer file '%s' to you.\n!!!If you want to receive this file, please input #y(#Y) or otherwise, input #n(#N)\n", pkt_read->client_id, pkt_read->file_name);
                                        // pkt_read->type = 4;
                                        DoRead();
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
                                        DoRead();
                                    }
                                    else if (pkt_read->type == 6) // cliet read client info leaved this room
                                    {
                                        // if(strcmp(pkt_read->client_id,client_id_.c_str()))
                                        std::cout << pkt_read->client_id << " has left this room." << std::endl;
                                        DoRead();
                                    }
                                }
                                else
                                {
                                    std::cerr << "Exception: " << ec.message() << "\n";

                                    socket_.close();
                                    exit(EXIT_FAILURE);
                                }
                            });
}

void ChatSession::DoWrite()
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
                                         DoWrite();
                                     }
                                 }
                                 else
                                 {
                                     std::cerr << "Exception: " << ec.message() << "\n";

                                     socket_.close();
                                 }
                             });
}