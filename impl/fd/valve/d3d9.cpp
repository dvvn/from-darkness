module;

#include <fd/object.h>

module fd.valve.d3d9;
import fd.rt_modules;

struct IDirect3DDevice9;

static auto _Init()
{
    using fd::rt_modules::shaderApiDx9;
    // @xref: "HandleLateCreation"
    const auto ptr  = shaderApiDx9.find_signature<"A1 ? ? ? ? 50 8B 08 FF 51 0C">();
    const auto ptr2 = reinterpret_cast<uintptr_t>(ptr) + 0x1;
    const auto d3d  = **reinterpret_cast<IDirect3DDevice9***>(ptr2);
    shaderApiDx9->log_class_info(d3d);
    return d3d;
}

FD_OBJECT_BIND(d3d_device9, _Init());
