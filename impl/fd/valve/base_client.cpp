module;

#include <fd/object.h>

module fd.valve.base_client;
import fd.rt_modules;
import fd.functional.invoke;

FD_OBJECT_IMPL(base_client, fd::rt_modules::client.find_interface<"VClient">());

bool base_client::DispatchUserMessage(int msg_type, int flags, int size, const void* msg)
{
    // return dhooks::invoke(&base_client::DispatchUserMessage, static_cast<size_t>(38), this, msg_type, flags, size, msg);
    auto vtable = *reinterpret_cast<void***>(this);
    auto fn     = vtable[38];
    return fd::invoke((decltype(&base_client::DispatchUserMessage)&)fn, this, msg_type, flags, size, msg);
}
