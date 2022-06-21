module;

#include <fd/core/object.h>

#include <functional>

module fd.csgo.interfaces.BaseClient;
import fd.rt_modules;

using namespace fd;
using namespace csgo;

FD_OBJECT_IMPL(IBaseClientDLL, 0, runtime_modules::client.find_interface<"VClient">());

bool IBaseClientDLL::DispatchUserMessage(int msg_type, int flags, int size, const void* msg)
{
    // return dhooks::invoke(&IBaseClientDLL::DispatchUserMessage, static_cast<size_t>(38), this, msg_type, flags, size, msg);
    const decltype(&IBaseClientDLL::DispatchUserMessage) fn = basic_address(this).deref<1>()[38];
    return std::invoke(fn, this, msg_type, flags, size, msg);
}
