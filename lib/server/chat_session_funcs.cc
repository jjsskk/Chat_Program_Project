#include "chat_session_server.h"

ChatSession::ChatSession(boost::asio::io_service &io_service, tcp::socket socket, list<shared_ptr<ChatRoom>> &roomlist, string port)
    : io_service_(io_service), socket_(move(socket)), port_(port),
      roomlist_(roomlist)
{
    // Start();
}
ChatSession::~ChatSession()
{
    free(pkt_);
    socket_.close();
    cout << "ChatSession terminated" << endl;
}

void ChatSession::Start()
{

    MakeRoomList();
    Deliver(*pkt_);
    DoRead();
    //  make_shared<ChatSession>(move(socket_), room_)->Start();
}
void ChatSession::Leave()
{
    // to_do remove ChatRoom in roomlist

    io_service_.post(
        [this]()
        {
            printf("restart\n");
            Start();
        });
}
struct packet *ChatSession::GetPacket()
{
    return pkt_;
}

void ChatSession::Deliver(struct packet &msg) //  해당 session으로 연결된 유저에게 msg 전송
{
    // fflush(stdout);
    printf("parcipant deliver :%s\n", msg.all_name);
    //  printf("hell3\n");
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.push_back(msg); // 전송할 msg(read받은 msg) 저장
    cout << write_in_progress << endl;
    if (!write_in_progress) // write_in progress =true(DoWrite()진행중) = write_msgs가 비어있지 않음
    {
        // printf("hell2\n");
        DoWrite();
    }
    // main thread 1개 기준으로 코드 짜짐
    // read가 msg가 들어 올때 마다 위의 Deliver()가 호출된다. 만약 DoWrite() 진행을
    //  다 마지치 못한 상황에서 비동기 read의 handler 호출로 현재 Deliver()가 호출되면
    // DoWrite()진행을 못마치고 온 상태라서 (write_in progress =true) DoWrite()를 다시 호출 못하게 막는다 (대신에 write_msgs_에 못보낸 read msg를 차곡차곡 쌓음)
    // 그후 아까 중단됬던 DoWrite를 다시 진행해서 write_msgs큐(그동안 못보낸 read msg가  큐에계속 쌓임)를 모두 보내서 모두 비우면 다시 read했을때 DoWrite()한다
}
void ChatSession::MakeRoomList()
{
    namelist_.clear();
    idlist_.clear();
    namelist_ = "";
    idlist_ = "";
    for (auto room : roomlist_)
    {

        namelist_ += room->GetRoomName();
        namelist_ += "/";
        idlist_ += to_string(room->GetRoomId());
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

void ChatSession::DoCreateRoom()
{
    string name(pkt_->selected_roomname);
    shared_ptr<ChatRoom> ptr =
        make_shared<ChatRoom>(room_id++, name);
    roomlist_.push_back(ptr);
    current_room_ = ptr;
    string client_id(pkt_->client_id);
    roomlist_.back()->JoinUser(client_id, shared_from_this()); // save client_id and socket in room
                                                                //  memset(pkt_->client_id,0,NAME_SIZE);
    pkt_->type = 2;
    // strcpy(pkt_->client_id,client_id_.c_str());
    roomlist_.back()->Deliver(*pkt_); // send new client name to all member in room ->print that 000 has joined in the room of all clients cmd
    memset(pkt_->all_name, 0, BUF_SIZE);
    pkt_->type = 1;
    strcpy(pkt_->all_name, roomlist_.back()->GetAllClientId(client_id_).c_str());
    Deliver(*pkt_); // send all user name to new client; ->dont save recent_msgs_ in chat room
                    //-> print that 000, welcome you in new client cmd

    // insert_newroom();
}

void ChatSession::DoEnterRoom()
{
    auto it = roomlist_.begin();
    for (; it != roomlist_.end(); it++)
        if (pkt_->selected_room_id == (*it)->GetRoomId())
        {
            string client_id(pkt_->client_id);
            current_room_ = *it;
            (*it)->JoinUser(client_id, shared_from_this());
            pkt_->type = 2;
            current_room_->Deliver(*pkt_); // send new client name to all member in room ->print that 000 has joined in the room of all clients cmd
            memset(pkt_->all_name, 0, BUF_SIZE);
            pkt_->type = 1;
            strcpy(pkt_->all_name, (*it)->GetAllClientId(client_id_).c_str());
            Deliver(*pkt_); // send all user name to new client; ->dont save recent_msgs_ in chat room
                            //-> print that 000, welcome you in new client cmd
            break;
        }
}

void ChatSession::NotifyCreatedRoom()
{
    // auto it = roomlist_.begin();
    for (auto participant : participants_life_)
    {
        if (participant == shared_from_this())
            continue;
        participant->MakeRoomList();
        participant->Deliver(*(participant->GetPacket()));
    }
}

void ChatSession::FileUpload(int thread_port)
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

        printf("writewrite!!\n");

        socket.write_some(boost::asio::buffer(file, read_length));
    }
    catch (exception &e)
    {
        cerr << "Exception: " << e.what() << "\n";
        socket.close();
    }

    printf(" file upload done\n");
    fclose(fp);
    fp = NULL;
    socket.close();
}