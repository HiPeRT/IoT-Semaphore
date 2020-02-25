/*
    C socket client example
*/

#include <stdio.h>
#include <string.h> //strlen
#include <stdlib.h> //strlen
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <pthread.h>   //for threading , link with lpthread
#include <sys/types.h>
#include <sys/stat.h>
#include <string>

#include <send.hpp>
#include "../support.h"


void prepare_message(Message *m, int idx)
{
    m->cam_idx = idx;
    m->t_stamp_ms = time_in_ms();
    m->num_objects = 5;    

    m->objects.clear();
    RoadUser r1{.3f,.4f,0,1,C_car};
    RoadUser r2{.5f,.6f,1,2,C_bus};
    RoadUser r3{.7f,.8f,2,3,C_bycicle};
    RoadUser r4{.9f,.10f,3,4,C_motorbike};
    RoadUser r5{.11f,.12f,4,5,C_person};
    m->objects.push_back(r1);
    m->objects.push_back(r2);
    m->objects.push_back(r3);
    m->objects.push_back(r4);
    m->objects.push_back(r5);

    m->lights.clear();
    TrafficLight l1{.1f,.2f,0,L_green,15};
    TrafficLight l2{.2f,.3f,2,L_red,15};
    m->lights.push_back(l1);
    m->lights.push_back(l2);

}

int main(int argc, char *argv[])
{

    Communicator Comm(SOCK_DGRAM);

    Comm.open_client_socket("127.0.0.1",8888);

    Message *m = new Message;

    for (int i=0; i<10; i++)
    {
        prepare_message(m,i);
        std::stringbuf s;
        Comm.serialize_coords(m,&s);
        
        std::cout<<s.str()<<std::endl;
        std::cout<<s.str().length()<<std::endl;
        
        Comm.send_message(m);
        sleep(1);
    }

    return 0;
}

