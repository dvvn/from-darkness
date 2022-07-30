module;

#include <fd/object.h>

module fd.valve.global_vars;
import fd.rt_modules;

namespace fd::valve
{
    struct base_client;
}

using namespace fd;
using namespace valve;

static auto _Init()
{
    // const void* addr       = rt_modules::client.find_signature<"A1 ? ? ? ? 5E 8B 40 10">();
    // const auto target_addr = **reinterpret_cast<global_vars_base***>((uintptr_t)addr + 0x1);

    // get it from CHLClient::HudUpdate @xref: "(time_int)", "(time_float)"
    const auto vtable      = *reinterpret_cast<void***>(&FD_OBJECT_GET(base_client*));
    const auto target_addr = **reinterpret_cast<global_vars_base***>((uintptr_t)vtable[11] + 0xA);
    on_class_found(rt_modules::client.data(), "class IGlobalVarsBase", target_addr);
    return target_addr;
}

FD_OBJECT_IMPL(global_vars_base*, _Init());
