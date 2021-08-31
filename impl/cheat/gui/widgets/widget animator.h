#pragma once

#include "cheat/gui/tools/animator.h"

#include "nstd/memory backup.h"

#define CHEAT_GUI_WIDGETS_FADE_CONTENT

namespace cheat::gui::widgets
{
	class widget_animator
	{
	protected:
		virtual ~widget_animator( );

		widget_animator(tools::animator&& a = { });

	public:
		bool animating( ) const;

	protected:
		virtual bool Animate( );
		float        Anim_value( ) const;

		tools::animator fade_;
	};

	class content_background_fader: public widget_animator
	{
	protected:
		content_background_fader(tools::animator&& a = { });

		bool Animate( ) final;

		nstd::memory_backup<float> fade_alpha_backup_;
	};
}
