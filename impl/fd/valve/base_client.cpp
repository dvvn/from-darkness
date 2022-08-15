module;

#include <fd/object.h>

module fd.valve.base_client;
import fd.rt_modules;
import fd.functional.invoke;

FD_OBJECT_IMPL(base_client, fd::rt_modules::client.find_interface<"VClient">());

bool base_client::DispatchUserMessage(int msg_type, int flags, int size, const void* msg)
{
    return fd::invoke_vfunc(&base_client::DispatchUserMessage, 38, this, msg_type, flags, size, msg);
}
