module;

#include <fd/core/object.h>

module fd.csgo.interfaces.Prediction;
import fd.rt_modules;

using namespace fd;
using namespace csgo;

FD_OBJECT_IMPL(IPrediction, 0, runtime_modules::client.find_interface<"VClientPrediction">());
