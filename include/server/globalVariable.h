#ifndef GLOVAL_VARIABLE
#define GLOVAL_VARIABLE
#include <set>
#include "chat_participant.h"

using namespace std;

extern set<chat_participant_ptr> participants_life_; // to manage client_session object life cycle
extern int room_id;
#endif