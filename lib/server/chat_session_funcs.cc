#include "chat_session_server.h"

ChatSession::ChatSession(boost::asio::io_service &io_service, tcp::socket socket, std::list<std::shared_ptr<ChatRoom>> &roomlist, std::string port)
    : io_service_(io_service), socket_(std::move(socket)), port_(port),
      roomlist_(roomlist)
{
    // start();
}
ChatSession::~ChatSession()
{
    free(pkt_);
    socket_.close();
    std::cout << "ChatSession terminated" << std::endl;
}

void ChatSession::start()
{

    make_roomlist();
    deliver(*pkt_);
    do_read();
    // do_read_room();
    //  std::make_shared<ChatSession>(std::move(socket_), room_)->start();
}
void ChatSession::leave()
{
    // to_do remove ChatRoom in roomlist

    io_service_.post(
        [this]()
        {
            printf("restart\n");
            start();
        });
}
struct packet *ChatSession::getPacket()
{
    return pkt_;
}

void ChatSession::deliver(struct packet &msg) // 채팅방 안 모든 유저에게 read받은 msg 전송
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
void ChatSession::make_roomlist()
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

void ChatSession::do_create_room()
{
    std::string name(pkt_->selected_roomname);
    std::shared_ptr<ChatRoom> ptr =
        std::make_shared<ChatRoom>(room_id++, name);
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

void ChatSession::do_enter_room()
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

void ChatSession::notify_created_room()
{
    // auto it = roomlist_.begin();
    for (auto participant : participants_life_)
    {
        if (participant == shared_from_this())
            continue;
        participant->make_roomlist();
        participant->deliver(*(participant->getPacket()));
    }
}

void ChatSession::file_upload(int thread_port)
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
        // int file_size = 0;
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