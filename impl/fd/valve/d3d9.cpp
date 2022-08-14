module;

#include <fd/object.h>

module fd.valve.d3d9;
import fd.rt_modules;

struct IDirect3DDevice9;

static auto _Init()
{
    using fd::rt_modules::shaderApiDx9;
    // @xref: "HandleLateCreation"
    const auto addr        = shaderApiDx9.find_signature<"A1 ? ? ? ? 50 8B 08 FF 51 0C">();
    const auto target_addr = **reinterpret_cast<IDirect3DDevice9***>((uintptr_t)addr + 0x1);
    shaderApiDx9->log_class_info<IDirect3DDevice9>(target_addr);
    return target_addr;
}

FD_OBJECT_BIND(d3d_device9, _Init());
