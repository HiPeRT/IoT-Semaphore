#include "MasaProtocolHandler.h"

unsigned long long MasaProtocolHandler::time_in_ms() {

    struct timeval tv;
    
    gettimeofday(&tv, NULL);
    
    unsigned long long t_stamp_ms = 
        (unsigned long long)(tv.tv_sec) * 1000 + (unsigned long long)(tv.tv_usec) / 1000;
    
    return t_stamp_ms;
}

MasaProtocolHandler::MasaProtocolHandler(std::string ip, int port) {
    
    _ip   = ip;
    _port = port;

    _comm = new Communicator(SOCK_DGRAM);

    _comm->open_client_socket(
        const_cast<char*>(_ip.c_str()), _port
    );
}
MasaProtocolHandler::~MasaProtocolHandler() {
    delete _comm;
}
Message * MasaProtocolHandler::prepare_message(std::vector<Semaphore *> S, uint32_t tlCountdown_sem[2], uint16_t tlState_sem[2]) {

    Message *m = new Message();

    m->t_stamp_ms = time_in_ms();

    uint16_t tlState;
    uint32_t tlCountdown;
    
    for(int i = 0; i < S.size(); ++i) {

        TrafficLight l;

        if(i % 2 == 0) {
            tlCountdown = tlCountdown_sem[0];
            tlState = tlState_sem[0];
        }
        else{
            tlCountdown = tlCountdown_sem[1];
            tlState = tlState_sem[1];
        }

        switch (tlState) {
        case SemaphoreState::Green:
            l.status = LightStatus::L_green;
            break;
        case SemaphoreState::Red:
            l.status = LightStatus::L_red;
            break;
        case SemaphoreState::Off:
        case SemaphoreState::Yellow:
        case SemaphoreState::Blinking:
        case SemaphoreState::GreenYellow:
            l.status = LightStatus::L_yellow;
            break;
        }

        l.latitude       = S[i]->getLatitude();
        l.longitude      = S[i]->getLongitude();
        l.orientation    = S[i]->getOrientation();
        l.time_to_change = tlCountdown;
        
        m->lights.push_back(l);
    }

    return m;
}
void MasaProtocolHandler::send_message(Message * m) {
    
    std::stringbuf s;

    _comm->serialize_coords(m, &s);
    _comm->send_message(m);

    delete m;
}