module;

#include <fd/object.h>

module fd.valve.client_state;
import fd.rt_modules;

FD_OBJECT_IMPL_HEAD(client_state*)
{
    using namespace fd::valve;
    using fd::rt_modules::engine;

    const auto ptr  = engine.find_signature<"A1 ? ? ? ? 8B 80 ? ? ? ? C3">();
    const auto ptr2 = reinterpret_cast<uintptr_t>(ptr) + 0x1;
    const auto cs   = **reinterpret_cast<client_state***>(ptr2);
    engine->log_class_info("class IClientState", cs);
    _Construct(cs);
}
