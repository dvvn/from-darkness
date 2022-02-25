module;

#include <cstring>

module cheat.csgo.interfaces.ModelRender;

using namespace cheat::csgo;

ModelRenderInfo_t::ModelRenderInfo_t( )
{
	std::memset(this, 0, sizeof(ModelRenderInfo_t));
}