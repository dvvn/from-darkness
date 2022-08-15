module;

#include <fd/object.h>

module fd.valve.weapon_system;
import fd.rt_modules;

FD_OBJECT_IMPL_HEAD(weapon_system)
{
    using fd::rt_modules::client;
    const auto ptr  = client.find_signature<"8B 35 ? ? ? ? FF 10 0F B7 C0">();
    const auto ptr2 = reinterpret_cast<uintptr_t>(ptr) + 0x2;
    const auto ws   = *reinterpret_cast<weapon_system**>(ptr2);
    client->log_class_info("class IWeaponSystem", ws);
    _Construct(ws);
}
