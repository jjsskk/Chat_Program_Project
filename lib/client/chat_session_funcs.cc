#include "chat_session_client.h"

ChatSession::ChatSession(boost::asio::io_service &io_service,
             tcp::resolver::iterator endpoint_iterator, std::string client_id, std::string host, std::string port)
    : io_service_(io_service),
      socket_(io_service), client_id_(client_id), host_(host), port_(port)
{
  DoConnect(endpoint_iterator);
}
ChatSession::~ChatSession()
{
  free(pkt_read);
  socket_.close();
}

void ChatSession::Close()
{
  io_service_.post([this]()
                   { socket_.close(); });
}

void ChatSession::FileUpload(int thread_port)
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

  Write(*pkt_file);
  free(pkt_file);
  pkt_file = NULL;
}
void ChatSession::Write(struct packet &msg)
{
  io_service_.post(
      [this, msg]()
      {
        bool write_in_progress = !write_msgs_.empty();
        write_msgs_.push_back(msg);
        if (!write_in_progress)
        {
          DoWrite();
        }
      });
}

void ChatSession::DisplayRoomList()
{
  std::cout << "Follow this guide line " << std::endl;
  std::cout << "1.input [q or Q] if you want to quit program  " << std::endl;
  std::cout << "2.input [create] if you want to create new room " << std::endl;
  std::cout << "3.input [existed only room Id not room Name] if you want to enter alread existed room" << std::endl;
  std::cout << "3.ex) [1] not [hello]" << std::endl;

  roomlist_.clear();
  roomidlist_.clear();
  char *ptr1 = strtok(pkt_read->all_name, "/");
  while (ptr1 != NULL)
  {
    roomlist_.emplace_back(ptr1);
    // printf("%s_%s\n", ptr1,ptr2);
    ptr1 = strtok(NULL, "/");
  }
  char *ptr2 = strtok(pkt_read->all_room_id, "/");
  while (ptr2 != NULL)
  {
    roomidlist_.emplace_back(ptr2);
    // printf("%s_%s\n", ptr1,ptr2);
    ptr2 = strtok(NULL, "/");
  }

  if (!strcmp(pkt_read->all_name, "There is no room!!"))
  {
    printf("%s\n", pkt_read->all_name);
    roomlist_.clear();
  }
  else
  {
    auto it = roomidlist_.begin();
    for (auto room : roomlist_)
    {
      std::cout << (*it) << "(roomId) ->"
                << "    " << room << "(roomName)" << std::endl;

      it++;
    }
  }
}
int ChatSession::UserinputInRoomList() // main thread execute this method
{
  DisplayRoomList();

  char line[20];
  int quit = 0;
  // int duplicated = 0;
  struct packet pkt_write;
  memset(line, 0, strlen(line));

  while (std::cin.getline(line, 20)) // remain places filled with null
  {
    if (!strcmp(line, "q") || !strcmp(line, "Q"))
    {
      Close();
      quit = 1;
      break;
    }
    if (!strcmp(line, "create"))
    {
      memset(line, 0, strlen(line));
      printf("enter room name : ");
      std::cin.getline(line, 20);

      std::cout << "Selectd room :" << line << std::endl;

      selectd_room_.clear();
      selectd_room_.append(line);
      memset(&pkt_write, 0, sizeof(struct packet));
      memcpy(pkt_write.selected_roomname, line, strlen(line));
      memcpy(pkt_write.client_id, client_id_.c_str(), client_id_.length());
      pkt_write.type = 0; // send created roomname and client id
      goto point;
    }
    auto it = roomlist_.begin();
    auto it_id = roomidlist_.begin();
    for (; it != roomlist_.end(); it++)
    {
      if (!strcmp(line, (*it_id).c_str()))
      {
        std::cout << "selectd room :" << *it << std::endl;
        // strcpy(selectd_room_.data(), line);
        selectd_room_.clear();
        selectd_room_.append(*it);
        memset(&pkt_write, 0, sizeof(struct packet));
        memcpy(pkt_write.selected_roomname, line, strlen(line));
        pkt_write.selected_room_id = atoi((*it_id).c_str());
        memcpy(pkt_write.client_id, client_id_.c_str(), client_id_.length());
        pkt_write.type = 1; // send existed roomname and client id
        goto point;
        // memcpy(reply.data(), line, strlen(line));
      }
      it_id++;
    }
    memset(line, 0, strlen(line));
    std::cout << "Pleasee enter existed chat room Id!" << std::endl;
  }
point:;
  // std::cout << "go out loop!" << std::endl;

  if (quit == 0)
  {
    printf("------------------------------\n");
    std::cout << "Welcome to " << selectd_room_ << " room!!!" << std::endl;
    std::cout << "1.Input [q or Q] if you want to leave this room." << std::endl;
    std::cout << "2.Input [#file] if you want to send file." << std::endl;
    std::cout << "3.when you receive msg about file transfer request from server." << std::endl;
    std::cout << "3.Input [#y] or [#Y] if you want to receive file. Otherwise, input [#n] or [#N]" << std::endl;
    Write(pkt_write);
    return 0;
  } // main thread
  else
  {
    pkt_write.type = 7;
    Write(pkt_write);
    // sleep(2);
    return 1;
  }
}
void ChatSession::DoCommunicateInRoom()
{
  //  printf("---------------\n");
  char line[BUF_SIZE];
  // std::cout << "Welcome to " << selectd_room_ << " room!!!" << std::endl;
  struct packet pkt_write;

  while (std::cin.getline(line, BUF_SIZE))
  { // n-1개의 문자 개수만큼 읽어와 str에 저장 (n번째 문자 or 나머지 문자전부는 NULL(‘\0’)로 바꾼다.)

    memset(&pkt_write, 0, sizeof(struct packet));
    pkt_write.type = 2; // send msg to server
    std::memcpy(pkt_write.client_id, client_id_.c_str(), client_id_.length());
    std::memcpy(pkt_write.msg, line, BUF_SIZE);
    if (!strcmp(line, "q") || !strcmp(line, "Q"))
    {
      pkt_write.type = 6;
      Write(pkt_write);
      break;
    }
    if (!strcmp(line, "#file"))
    {
      int num = 0;

      do
      {
        num = 0;
        printf("enter file name : ");
        std::cin.getline(line, BUF_SIZE);
        fp = fopen(line, "rb");
        if (fp == NULL)
        {
          printf("no such a file\n");
          num = 1;
          // exit(1);
        }
      } while (num == 1);
      memset(pkt_write.file_name, 0, NAME_SIZE);
      memset(pkt_write.client_id, 0, NAME_SIZE);
      strcpy(pkt_write.file_name, line);
      strcpy(file_name_, line);
      strcpy(pkt_write.client_id, client_id_.c_str());
      pkt_write.type = 3;
      std::srand(std::time(NULL));
      // std::cout << "rand : " << rand() << std::endl;
      int port = 0;
      do
      {
        port = rand() % 400 + 1;

      } while ((port + 10000) == atoi(port_.c_str()));
      pkt_write.port = port;
      Write(pkt_write);
      std::thread t(&ChatSession::FileUpload, this, port);
      t.detach();
      // printf("here");

      goto point;
    }
    while (file == 1)
    {

      if (!strcmp(line, "#y") || !strcmp(line, "#Y"))
      {
        memset(pkt_write.file_name, 0, NAME_SIZE);
        strcpy(pkt_write.file_name, file_name_);
        pkt_write.file_transfer_check = 1;
        pkt_write.type = 5;
        Write(pkt_write);
        // std::thread t(&ChatSession::file_download, this);
        // t.detach();
        // file_transfer();
        file = 0;
        goto point;
      }
      else if (!strcmp(line, "#n") || !strcmp(line, "#N"))
      {
        pkt_write.file_transfer_check = 0;
        pkt_write.type = 5;
        Write(pkt_write);
        file = 0;
        goto point;
      }
      memset(line, 0, strlen(line));
      printf("please input #y(#Y) or #n(#N) : ");
      std::cin.getline(line, BUF_SIZE);
    }
    pkt_write.type = 2;
    Write(pkt_write);
  // printf("here\n");
  point:;
    memset(line, 0, strlen(line));
  }
  // DoRead();
}