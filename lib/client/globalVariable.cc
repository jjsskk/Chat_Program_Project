#include "globalVariable.h"

#define BUF_SIZE 100
#define NAME_SIZE 20
#define FILE_SIZE 500


std::mutex mtx_list;
std::mutex mtx_room;
std::mutex mtx_file;
// std::mutex mtx_room_out;
int list = 0;
int room = 0;
int file = 0;