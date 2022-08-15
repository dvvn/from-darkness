module;

#include <fd/object.h>

module fd.valve.client_mode;
import fd.rt_modules;

namespace fd::valve
{
    struct base_client;
}

FD_OBJECT_IMPL_HEAD(client_mode_shared)
{
    using namespace fd::valve;
    using fd::rt_modules::client;

    const auto vtable = *reinterpret_cast<uintptr_t**>(&FD_OBJECT_GET(base_client*));
    const auto ptr    = vtable[10] + 0x5; // get it from CHLClient::HudProcessInput
    const auto cm     = **reinterpret_cast<client_mode_shared***>(ptr);
    client->log_class_info("class IClientModeShared", cm);
    _Construct(cm);
}
