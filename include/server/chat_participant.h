#ifndef CHAT_PARTICIPANT_H
#define CHAT_PARTICIPANT_H
#include <memory>
#include "struct_pkt.h"
class ChatParticipant
{
public:
    virtual ~ChatParticipant() {}
    virtual void deliver(struct packet &msg) = 0;
    virtual void make_roomlist() = 0;
    virtual struct packet *getPacket() = 0;
    // virtual std::string get_client_id() =0;
};

typedef std::shared_ptr<ChatParticipant> chat_participant_ptr;

#endif