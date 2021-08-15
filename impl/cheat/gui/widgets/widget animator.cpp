#include "widget animator.h"

using namespace cheat;
using namespace gui::widgets;
using namespace gui::tools;
using namespace utl;

widget_animator::~widget_animator( ) = default;

widget_animator::widget_animator(animator&& a): fade_(std::move(a))
{
}

bool widget_animator::Animate( )
{
	return fade_.update( );
}

bool widget_animator::animating( ) const
{
	return fade_.updating( );
}

float widget_animator::Anim_value( ) const
{
	return fade_.value( );
}

content_background_fader::content_background_fader(animator&& a): widget_animator(std::move(a))
{
}

bool content_background_fader::Animate( )
{
	runtime_assert(!fade_alpha_backup_);
	if (widget_animator::Animate( ))
	{
		fade_alpha_backup_ = memory_backup(ImGui::GetStyle( ).Alpha, fade_.value( ));
		return true;
	}
	return false;
}
