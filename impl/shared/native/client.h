#pragma once
#include "client_class.h"
#include ".detail/interface.h"

namespace fd
{
enum class native_frame_stage : int32_t
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

FD_BIND_NATIVE_INTERFACE(CHLClient, client);

union native_client
{
    FD_NATIVE_INTERFACE(CHLClient);
    function<8, native_client_class *> get_all_classes;
};
}