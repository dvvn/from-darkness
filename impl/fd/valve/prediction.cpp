module;

#include <fd/object.h>

module fd.valve.prediction;
import fd.rt_modules;

FD_OBJECT_IMPL(prediction, fd::rt_modules::client.find_interface<"VClientPrediction">());

FD_OBJECT_IMPL_HEAD(move_helper)
{
    using fd::rt_modules::client;
    const auto ptr  = client.find_signature<"8B 0D ? ? ? ? 8B 45 ? 51 8B D4 89 02 8B 01">();
    const auto ptr2 = reinterpret_cast<uintptr_t>(ptr) + 0x2;
    const auto mh   = **reinterpret_cast<move_helper***>(ptr2);
    client->log_class_info("class IMoveHelper", mh);
    _Construct(mh);
}
