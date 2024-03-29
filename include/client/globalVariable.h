#ifndef GLOVAL_VARIABLE 
#define GLOVAL_VARIABLE
#include <mutex>

#define BUF_SIZE 100
#define NAME_SIZE 20
#define FILE_SIZE 500
using namespace std;

struct packet
{
    short type;
    char client_id[NAME_SIZE];
    char selected_roomname[NAME_SIZE];
    int selected_room_id;
    char msg[BUF_SIZE];
    char all_name[BUF_SIZE]; // used both in all room name and in all user name
    char all_room_id[BUF_SIZE];
    short file_transfer_check;
    char file_name[NAME_SIZE];
    int file_size;
    char file_content[FILE_SIZE];
    int end_read_size;
    int port;
};
extern mutex mtx_list;
extern mutex mtx_room;
extern mutex mtx_file;
extern int list;
extern int room;
extern int file;

#endif