module;

#include <fd/object.h>

module fd.valve.prediction;
import fd.rt_modules;

FD_OBJECT_IMPL(prediction, 0, fd::runtime_modules::client.find_interface<"VClientPrediction">());
FD_OBJECT_IMPL(move_helper, 0, fd::runtime_modules::client.find_interface_sig<"8B 0D ? ? ? ? 8B 45 ? 51 8B D4 89 02 8B 01">().plus(2).deref<2>());
