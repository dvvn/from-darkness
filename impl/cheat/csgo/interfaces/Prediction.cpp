module;

#include <cheat/csgo/interface.h>

module cheat.csgo.interfaces.Prediction;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_CSGO_INTERFACE_INIT(IPrediction, csgo_modules::client.find_interface<"VClientPrediction">( ));