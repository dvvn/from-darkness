module;

#include <fd/object.h>

module fd.valve.local_player;
import fd.rt_modules;

using local_player_t = decltype(local_player)::value_type;

FD_OBJECT_IMPL_HEAD(local_player_t)
{
    using fd::rt_modules::client;
    const auto ptr  = client.find_signature<"8B 0D ? ? ? ? 83 FF FF 74 07">();
    const auto ptr2 = reinterpret_cast<uintptr_t>(ptr) + 0x2;
    const auto lp   = *reinterpret_cast<local_player_t*>(ptr2);
    client->log_class_info("LocalPlayer", lp);
    _Construct(lp);
}
