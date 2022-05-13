module;

#include <cheat/tools/interface.h>

module cheat.csgo.interfaces.Prediction;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_INTERFACE_IMPL(IPrediction, csgo_modules::client.find_interface<"VClientPrediction">( ));