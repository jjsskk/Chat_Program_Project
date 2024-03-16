#include "globalVariable.h"

#define BUF_SIZE 100
#define NAME_SIZE 20
#define FILE_SIZE 500


mutex mtx_list;
mutex mtx_room;
mutex mtx_file;
// mutex mtx_room_out;
int list = 0;
int room = 0;
int file = 0;