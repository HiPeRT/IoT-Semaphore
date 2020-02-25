#include "serialize.hpp"
#include <time.h>

struct obj_coords
{
    float LAT;
    float LONG;
    float cl;
};

unsigned long long time_in_ms()
{    
    struct timeval tv;
    gettimeofday(&tv, NULL);
    unsigned long long t_stamp_ms = (unsigned long long)(tv.tv_sec) * 1000 + (unsigned long long)(tv.tv_usec) / 1000;
    return t_stamp_ms;
}

void create_Message(struct obj_coords *c, int obj_n, int CAM_IDX, Message *m)
{   
    std::vector<RoadUser> ruv;
    int i;
    for (i = 0; i < obj_n; i++)
    {
        RoadUser r{c[i].LAT,c[i].LONG,0,0,(Categories)c[i].cl};
        ruv.push_back(r);
    }
    m->cam_idx = CAM_IDX;
    m->t_stamp_ms = time_in_ms();
    m->num_objects = ruv.size();
    m->objects = ruv;    
}