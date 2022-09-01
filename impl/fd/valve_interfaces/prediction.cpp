module;

#include <fd/object.h>

module fd.valve.prediction;
import fd.rt_modules;

using fd::rt_modules::client;

FD_OBJECT_IMPL(prediction, client.find_interface<"VClientPrediction">());
FD_OBJECT_IMPL(move_helper, (client.find_interface_sig<"8B 0D ? ? ? ? 8B 45 ? 51 8B D4 89 02 8B 01", 0x2, 2, "class IMoveHelper">()));
