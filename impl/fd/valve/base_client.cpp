module;

#include <fd/object.h>

module fd.valve.base_client;
import fd.rt_modules;
import fd.functional;

FD_OBJECT_IMPL(base_client, fd::rt_modules::client.find_interface<"VClient">());

bool base_client::DispatchUserMessage(int msg_type, int flags, int size, const void* msg)
{
    // return dhooks::invoke(&base_client::DispatchUserMessage, static_cast<size_t>(38), this, msg_type, flags, size, msg);
    const decltype(&base_client::DispatchUserMessage) fn = fd::basic_address(this).deref<1>()[38];
    return fd::invoke(fn, this, msg_type, flags, size, msg);
}
