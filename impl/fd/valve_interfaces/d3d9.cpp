#include <fd/object.h>

import fd.rt_modules;

struct IDirect3DDevice9;

FD_OBJECT_IMPL_HEAD(IDirect3DDevice9*)
{
    using fd::rt_modules::shaderApiDx9;
    // @xref: "HandleLateCreation"
    const auto ptr  = shaderApiDx9.find_signature<"A1 ? ? ? ? 50 8B 08 FF 51 0C">();
    const auto ptr2 = reinterpret_cast<uintptr_t>(ptr) + 0x1;
    const auto d3d  = **reinterpret_cast<IDirect3DDevice9***>(ptr2);
    shaderApiDx9->log_class_info(d3d);
    _Construct(d3d);
}
