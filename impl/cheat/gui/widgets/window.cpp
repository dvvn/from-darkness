#include "window.h"

using namespace cheat;
using namespace gui;
using namespace widgets;
using namespace tools;
using namespace utl;

window::window(animator&& fade) : fade__(move(fade))
{
}

bool window::begin(const string_wrapper& title, ImGuiWindowFlags_ flags)
{
	BOOST_ASSERT(ignore_end__ == false);

	const auto animate = fade__.update( );
	if (!animate && !visible__)
	{
		ignore_end__ = true;
		return false;
	}

	auto& style = ImGui::GetStyle( );

	BOOST_ASSERT(style.Alpha == fade__.max( ));

#ifndef CHEAT_GUI_WINDOW_FADE_CONTENT
	memory_backup<float> alpha_backup;
	(void)alpha_backup;
#endif
	if (animate)
	{
#ifndef CHEAT_GUI_WINDOW_FADE_CONTENT
		alpha_backup
#else
		BOOST_ASSERT(!fade_alpha_backup__);
		fade_alpha_backup__
#endif
				= memory_backup(style.Alpha, fade__.value( ));
	}

	constexpr auto dummy_text = string_view("W");
	const auto sample_size = ImGui::CalcTextSize(dummy_text._Unchecked_begin( ), dummy_text._Unchecked_end( ));
	const auto min_size = ImGui::GetFontSize( ) + //small button size
						  style.ItemInnerSpacing.x +
						  sample_size.x * title.raw( ).size( ) + //string size
						  style.FramePadding.x * 2.f +           //space between and after
						  style.WindowBorderSize;
	//ImGui::SetNextWindowContentSize({min_size, 0});

	memory_backup<float> min_size_backup;
	(void)min_size_backup;

	if (min_size > style.WindowMinSize.x)
		min_size_backup = memory_backup(style.WindowMinSize.x, min_size);

	return ImGui::Begin(title, nullptr, flags);
}

void window::end( )
{
	if (ignore_end__)
	{
		ignore_end__ = false;
#ifdef CHEAT_GUI_WINDOW_FADE_CONTENT
		BOOST_ASSERT(!fade_alpha_backup__);
#endif
	}
	else
	{
		ImGui::End( );
#ifdef CHEAT_GUI_WINDOW_FADE_CONTENT
		if (fade_alpha_backup__)
			fade_alpha_backup__ = { };
#endif
	}
}

void window::show( )
{
	visible__ = true;
	fade__.set(1);
}

void window::hide( )
{
	visible__ = false;
	fade__.set(-1);
}

void window::toggle( )
{
	visible__ = !visible__;
	fade__.set(visible__ ? 1 : -1);
}

bool window::visible( ) const
{
	return visible__ || animating( );
}

bool window::animating( ) const
{
	return fade__.updating( );
}

bool window::active( ) const
{
	return visible__ && fade__.done(1);
}
