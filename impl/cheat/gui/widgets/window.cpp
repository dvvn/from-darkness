#include "window.h"

#include "cheat/gui/tools/info.h"
#include "cheat/gui/tools/push style color.h"
#include "cheat/gui/tools/push style var.h"

using namespace cheat;
using namespace gui;
using namespace widgets;
using namespace tools;
using namespace utl;

window::window(animator&& fade) : content_background_fader(move(fade))
{
}

bool window::begin(prefect_string&& title, ImGuiWindowFlags_ flags)
{
	BOOST_ASSERT(ignore_end__ == false);
	auto& style = ImGui::GetStyle( );
	BOOST_ASSERT(style.Alpha == fade_.max( ));

	if (!this->Animate( ) && !visible__)
	{
		ignore_end__ = true;
		return false;
	}

#ifndef CHEAT_GUI_WIDGETS_FADE_CONTENT
	auto alpha_backup = move(fade_alpha_backup_);
	(void)alpha_backup;
#endif

	const auto min_size = ImGui::GetFontSize( ) + //small button size
						  style.ItemInnerSpacing.x +
						  _Get_char_size( ).x * title.size( ) + //string size
						  style.FramePadding.x * 2.f +          //space between and after
						  style.WindowBorderSize;

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
#ifdef CHEAT_GUI_WIDGETS_FADE_CONTENT
		BOOST_ASSERT(!fade_alpha_backup_);
#endif
	}
	else
	{
		ImGui::End( );
#ifdef CHEAT_GUI_WIDGETS_FADE_CONTENT
		fade_alpha_backup_.restore( );
#endif
	}
}

void window::show( )
{
	visible__ = true;
	fade_.set(1);
}

void window::hide( )
{
	visible__ = false;
	fade_.set(-1);
}

void window::toggle( )
{
	visible__ = !visible__;
	fade_.set(visible__ ? 1 : -1);
}

bool window::visible( ) const
{
	return visible__ || animating( );
}

bool window::active( ) const
{
	return visible__ && fade_.done(1);
}

child_window::child_window(animator&& fade): content_background_fader(move(fade))
{
}

bool child_window::Begin_impl(ImGuiID id, const ImVec2& size_arg, bool border, ImGuiWindowFlags extra_flags)
{
	return ImGui::BeginChild(id, size_arg, border, extra_flags);
}

bool child_window::begin(const size_info& size_info_x, const size_info& size_info_y, bool border, ImGuiWindowFlags_ flags)
{
	const auto& style = ImGui::GetStyle( );
	this->Animate( );

#ifndef CHEAT_GUI_WIDGETS_FADE_CONTENT
	auto alpha_backup = move(fade_alpha_backup_);
	(void)alpha_backup;
#endif

	auto&& sample_size = _Get_char_size( );

	//const auto indent_headers = style.ItemInnerSpacing.x;
	//const auto indent_page = style./*ItemInnerSpacing*/ItemSpacing.x; //ImGui use ItemSpacing by default

	const auto frame_padding = style.FramePadding * 2.f;

	const auto size_x = [&]( )-> ImVec2
	{
		if (size_info_x.type == size_info::UNSET)
			return { };

		const auto reserve_x = [&]( )-> float
		{
			switch (size_info_x.type)
			{
				case size_info::WORD: return (size_info_x.biggest_element * sample_size.x) * size_info_x.count;
				case size_info::RAW: return (size_info_x.biggest_element) * size_info_x.count;
				default: throw;
			}
		};
		const auto reserve_y = [&]( )-> float
		{
			switch (size_info_x.type)
			{
				case size_info::WORD: return sample_size.y;
				case size_info::RAW: return ImGui::GetFrameHeight( );
				default: throw;
			}
		};

		ImVec2 size;

		size.x = frame_padding.x +                              //space before and after
				 /*indent_headers +*/                           //to indent first selectable
				 (reserve_x( )) +                               //reserve width for all strings / whole data
				 style.ItemSpacing.x * (size_info_x.count - 1); //space between all headers

		size.y = frame_padding.y + //space before and after
				 reserve_y( );     //word height

		return size;
	}( );
	const auto size_y = [&]( )-> ImVec2
	{
		if (size_info_y.type == size_info::UNSET)
			return { };

		const auto reserve_x = [&]( )-> float
		{
			switch (size_info_y.type)
			{
				case size_info::WORD: return (size_info_y.biggest_element * sample_size.x);
				case size_info::RAW: return (size_info_y.biggest_element);
				default: throw;
			}
		};
		const auto reserve_y = [&]( )-> float
		{
			switch (size_info_y.type) //ImGui::GetFrameHeight( );
			{
				case size_info::WORD: return (size_info_y.count * sample_size.y);
				case size_info::RAW: return size_info_y.count * ImGui::GetFrameHeight( );
				default: throw;
			}
		};

		ImVec2 size;
		size.x = frame_padding.x + //space before and after
				 reserve_x( );     //reserve width for longest string

		size.y = frame_padding.y +                              //space before and after
				 reserve_y( ) +                                 //all strings height						                            
				 style.ItemSpacing.y * (size_info_y.count - 1); //space between all string

		return size;
	}( );

	const ImVec2 size = {std::max(size_x.x, size_y.x), std::max(size_x.y, size_y.y)};

	//return (ImGui::BeginChildFrame(reinterpret_cast<ImGuiID>(this), size, flags));
	//return ImGui::BeginChild(reinterpret_cast<ImGuiID>(this), size, true, flags);

	return Begin_impl(reinterpret_cast<ImGuiID>(this), size, border, flags);
}

void child_window::end( )
{
	ImGui::EndChild( );
#ifdef CHEAT_GUI_WIDGETS_FADE_CONTENT
	if (fade_alpha_backup_)
		fade_alpha_backup_ = { };
#endif
}

void child_window::show( )
{
	return fade_.set(1);
}

child_frame_window::child_frame_window(animator&& fade): child_window(move(fade))
{
}

bool child_frame_window::Begin_impl(ImGuiID id, const ImVec2& size_arg, bool border, ImGuiWindowFlags extra_flags)
{
	const auto& style = ImGui::GetStyle( );

	auto backups = make_tuple(push_style_color(ImGuiCol_ChildBg, style.Colors[ImGuiCol_FrameBg]),
							  push_style_var(ImGuiStyleVar_ChildRounding, style.FrameRounding),
							  push_style_var(ImGuiStyleVar_ChildBorderSize, style.FrameBorderSize),
							  push_style_var(ImGuiStyleVar_WindowPadding, style.FramePadding));;
	(void)backups;
	return child_window::Begin_impl(id, size_arg, border, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysUseWindowPadding | extra_flags);
}
