module;

module cheat.csgo.interfaces.RenderView;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

IVRenderView* nstd::one_instance_getter<IVRenderView*>::_Construct( )const
{
	return csgo_modules::engine->find_game_interface("VEngineRenderView");
}