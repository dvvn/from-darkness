module;

#include <cheat/csgo/interface.h>

module cheat.csgo.interfaces.GlowEffect;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_CSGO_INTERFACE_INIT(CGlowObjectManager)
{
	CGlowObjectManager* const ret = csgo_modules::client.find_signature<"0F 11 05 ? ? ? ? 83 C8 01">( ).plus(3).deref<1>( );
	csgo_modules::client.log_found_interface(ret);
	return ret;
}

void GlowObject_t::Set(const Color& glow_color, GlowRenderStyles style)
{
	this->color = {static_cast<float>(glow_color[0]), static_cast<float>(glow_color[1]), static_cast<float>(glow_color[2]), static_cast<float>(glow_color[3])};
	this->bloom_amount = 1.0f;
	this->render_when_occluded = true;
	this->render_when_unoccluded = false;
	this->render_style = style;
}

bool GlowObject_t::IsEmpty( ) const
{
	return next_free_slot != GLOW_ENTRY_IN_USE;
}
