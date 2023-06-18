#pragma once
#include <fd/abstract_interface.h>

namespace fd::valve
{
struct client_class
{
    void *create;
    void *create_event;
    char const *name;
    struct recv_table *table;
    client_class *next;
    uint32_t id;
};

enum class frame_stage : int32_t
{
    undefined = -1,
    start,
    // a network packet is being received
    net_update_start,
    // data has been received and we are going to start calling postdataupdate
    net_update_postdataupdate_start,
    // data has been received and called postdataupdate on all data recipients
    net_update_postdataupdate_end,
    // received all packets, we can now do interpolation, prediction, etc
    net_update_end,
    // start rendering the scene
    render_start,
    // finished rendering the scene
    render_end,
    /*net_full_frame_update_on_remove*/
};

union client
{
    FD_ABSTRACT_INTERFACE(client);
    abstract_function<8, client_class *> get_all_classes;
};
}