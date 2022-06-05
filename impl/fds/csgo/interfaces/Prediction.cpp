module;

#include <fds/tools/interface.h>

module fds.csgo.interfaces.Prediction;
import fds.csgo.modules;

using namespace fds;
using namespace csgo;

FDS_INTERFACE_IMPL(IPrediction, csgo_modules::client.find_interface<"VClientPrediction">());
