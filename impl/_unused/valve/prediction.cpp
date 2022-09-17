module;

#include <fd/object.h>

module fd.valve.prediction;
import fd.rt_modules;

using namespace fd::rt_modules;

FD_OBJECT_ATTACH_EX(prediction, client_fn().find_interface<"VClientPrediction">());
FD_OBJECT_ATTACH_EX(move_helper, client_fn().find_interface_sig<"8B 0D ? ? ? ? 8B 45 ? 51 8B D4 89 02 8B 01", 0x2, 2, "class IMoveHelper">());
