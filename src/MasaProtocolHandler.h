#ifndef MASAPROTOCOL_H
#define MASAPROTOCOL_H

#include "../MASA_protocol/include/send.hpp"
#include "Semaphore.h"

class MasaProtocolHandler{

    public:

    MasaProtocolHandler(std::string ip, int port);
    ~MasaProtocolHandler();
    Message * prepare_message(std::vector<Semaphore *> S, uint32_t tlCountdown[2], uint16_t tlState[2]);
    void send_message(Message * m);

    protected:
    unsigned long long time_in_ms();

    private:

    std::string _ip;
    int _port;

    Communicator *_comm;
};

#endif