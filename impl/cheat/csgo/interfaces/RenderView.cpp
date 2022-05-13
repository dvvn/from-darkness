module;

#include <cheat/tools/interface.h>

module cheat.csgo.interfaces.RenderView;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_INTERFACE_IMPL(IVRenderView, csgo_modules::engine.find_interface<"VEngineRenderView">( ));