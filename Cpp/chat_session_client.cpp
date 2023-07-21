void chat_session::write(struct packet &msg)
{
  io_service_.post(
      [this, msg]()
      {
        bool write_in_progress = !write_msgs_.empty();
        write_msgs_.push_back(msg);
        if (!write_in_progress)
        {
          do_write();
        }
      });
}

void chat_session::display_roomlist()
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
int chat_session::userinput_in_roomlist() // main thread execute this method
{
  display_roomlist();

  char line[20];
  int quit = 0;
  // int duplicated = 0;
  struct packet pkt_write;
  memset(line, 0, strlen(line));

  while (std::cin.getline(line, 20)) // remain places filled with null
  {
    if (!strcmp(line, "q") || !strcmp(line, "Q"))
    {
      close();
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
    write(pkt_write);
    return 0;
  } // main thread
  else
  {
    pkt_write.type = 7;
    write(pkt_write);
    // sleep(2);
    return 1;
  }
}
void chat_session::do_communicate_in_room()
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
      write(pkt_write);
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
      write(pkt_write);
      std::thread t(&chat_session::file_upload, this, port);
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
        write(pkt_write);
        // std::thread t(&chat_session::file_download, this);
        // t.detach();
        // file_transfer();
        file = 0;
        goto point;
      }
      else if (!strcmp(line, "#n") || !strcmp(line, "#N"))
      {
        pkt_write.file_transfer_check = 0;
        pkt_write.type = 5;
        write(pkt_write);
        file = 0;
        goto point;
      }
      memset(line, 0, strlen(line));
      printf("please input #y(#Y) or #n(#N) : ");
      std::cin.getline(line, BUF_SIZE);
    }
    pkt_write.type = 2;
    write(pkt_write);
  // printf("here\n");
  point:;
    memset(line, 0, strlen(line));
  }
  // do_read();
}