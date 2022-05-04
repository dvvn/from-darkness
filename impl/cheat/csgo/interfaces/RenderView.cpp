module;

#include <cheat/csgo/interface.h>

module cheat.csgo.interfaces.RenderView;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_CSGO_INTERFACE_INIT(IVRenderView, csgo_modules::engine.find_interface<"VEngineRenderView">( ));