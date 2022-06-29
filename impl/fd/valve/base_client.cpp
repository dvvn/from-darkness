module;

#include <fd/object.h>

#include <functional>

module fd.valve.base_client;
import fd.rt_modules;

FD_OBJECT_IMPL(base_client, 0, fd::runtime_modules::client.find_interface<"VClient">());

bool base_client::DispatchUserMessage(int msg_type, int flags, int size, const void* msg)
{
    // return dhooks::invoke(&base_client::DispatchUserMessage, static_cast<size_t>(38), this, msg_type, flags, size, msg);
    const decltype(&base_client::DispatchUserMessage) fn = fd::basic_address(this).deref<1>()[38];
    return std::invoke(fn, this, msg_type, flags, size, msg);
}
