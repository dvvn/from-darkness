module cheat.csgo.interfaces:GlowEffect;

using namespace cheat::csgo;

void GlowObject_t::Set(const Color& glow_color, GlowRenderStyles style)
{
	this->color                  = {static_cast<float>(glow_color[0]), static_cast<float>(glow_color[1]), static_cast<float>(glow_color[2]), static_cast<float>(glow_color[3])};
	this->bloom_amount           = 1.0f;
	this->render_when_occluded   = true;
	this->render_when_unoccluded = false;
	this->render_style           = style;
}

bool GlowObject_t::IsEmpty( ) const
{
	return next_free_slot != GLOW_ENTRY_IN_USE;
}
