module;

#include <fd/object.h>

module fd.valve.global_vars;
import fd.rt_modules;

namespace fd::valve
{
    struct base_client;
}

FD_OBJECT_ATTACH_MANUAL(global_vars_base*)
{
    using namespace fd;
    using namespace valve;

    // const void* addr       = rt_modules::client.find_signature<"A1 ? ? ? ? 5E 8B 40 10">();
    // const auto target_addr = **reinterpret_cast<global_vars_base***>((uintptr_t)addr + 0x1);

    // get it from CHLClient::HudUpdate
    //@xref: "(time_int)", "(time_float)"
    const auto vtable      = *reinterpret_cast<uintptr_t**>(&FD_OBJECT_GET(base_client*));
    const auto target_addr = **reinterpret_cast<global_vars_base***>(vtable[11] + 0xA);
    rt_modules::client->log_class_info("class IGlobalVarsBase", target_addr);
    return target_addr;
}
