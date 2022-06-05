module;

#include <fds/tools/interface.h>

#include <functional>

module fds.csgo.interfaces.BaseClient;
import fds.csgo.modules;

using namespace fds;
using namespace csgo;

FDS_INTERFACE_IMPL(IBaseClientDLL, csgo_modules::client.find_interface<"VClient">());

bool IBaseClientDLL::DispatchUserMessage(int msg_type, int flags, int size, const void* msg)
{
    // return dhooks::invoke(&IBaseClientDLL::DispatchUserMessage, static_cast<size_t>(38), this, msg_type, flags, size, msg);
    const decltype(&IBaseClientDLL::DispatchUserMessage) fn = nstd::mem::basic_address(this).deref<1>()[38];
    return std::invoke(fn, this, msg_type, flags, size, msg);
}
