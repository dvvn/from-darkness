module;

#include <fd/object.h>

module fd.valve.client_state;
import fd.rt_modules;

FD_OBJECT_IMPL_HEAD(client_state*)
{
    _Construct(fd::rt_modules::engine.find_interface_sig<"A1 ? ? ? ? 8B 80 ? ? ? ? C3", 0x1, 2, "class IClientState">());
}
