module;

#include <fd/object.h>

module fd.valve.base_client;
import fd.rt_modules;
import fd.functional.invoke;

using namespace fd;

FD_OBJECT_ATTACH_EX(base_client, rt_modules::client_fn().find_interface<"VClient">());

bool base_client::DispatchUserMessage(int msg_type, int flags, int size, const void* msg)
{
    return invoke(&base_client::DispatchUserMessage, 38, this, msg_type, flags, size, msg);
}
