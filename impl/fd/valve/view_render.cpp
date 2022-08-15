module;

#include <fd/object.h>

module fd.valve.view_render;
import fd.rt_modules;

FD_OBJECT_IMPL_HEAD(view_render)
{
    using fd::rt_modules::client;
    const auto ptr  = client.find_signature<"8B 0D ? ? ? ? FF 75 0C 8B 45 08">();
    const auto ptr2 = reinterpret_cast<uintptr_t>(ptr) + 0x2;
    const auto vr   = **reinterpret_cast<view_render***>(ptr2);
    client->log_class_info("class IViewRender", vr);
    _Construct(vr);
}
