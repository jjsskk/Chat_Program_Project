#include <cstdlib>
#include <deque>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;
#include "globalVariable.h"

#include "chat_session_client.h"

using work_guard_type = boost::asio::executor_work_guard<boost::asio::io_service::executor_type>;

int main(int argc, char *argv[])
{
  try
  {
    if (argc != 4)
    {
      std::cerr << "Usage: chat_client <host> <port> <ID>\n";
      return 1;
    }

    boost::asio::io_service io_service;
    work_guard_type work_guard(io_service.get_executor()); 
    // to prevent ioservice terminated when thread assigned to ioservice have no work
    tcp::resolver resolver(io_service);
    auto endpoint_iterator = resolver.resolve({argv[1], argv[2]});
    ChatSession c(io_service, endpoint_iterator, argv[3], argv[1], argv[2]);
    std::thread t([&io_service]()
                  { io_service.run(); });

    while (1)
    {
      while (list == 0) // globla variable
      {
      }
      // printf("display\n");
      printf("----------------------------------------------\n");
      if (c.userinput_in_roomlist() == 1)
        break;
      mtx_list.lock();
      list = 0;
      mtx_list.unlock();
      //  t.join();
      //   printf("what the fuck\n");
      while (room == 0) // globla variable
      {
        // printf("0");
      }
      // printf("communicate\n");
      c.do_communicate_in_room();
      mtx_room.lock();
      room = 0;
      mtx_room.unlock();
    }

    // c.close();
    io_service.stop();
    t.join();
    std::cout << "program terminated" << std::endl;
  }
  catch (std::exception &e)
  {
    std::cerr << "Exception: " << e.what() << "!!!"
              << "\n";
  }

  return 0;
}