#include "window.h"

#if 0
#include "cheat/gui/tools/info.h"
#include "cheat/gui/tools/push style color.h"
#include "cheat/gui/tools/push style var.h"
#include "cheat/gui/tools/cached_text.h"

#include <nstd/runtime assert.h>

#include <imgui_internal.h>

using namespace cheat;
using namespace gui;
using namespace widgets;
using namespace tools;

window::window(animator&& fade)
	: content_background_fader(std::move(fade))
{
}

bool window::begin(imgui_string_transparent&& title, ImGuiWindowFlags_ flags)
{
	runtime_assert(ignore_end__ == false);
	auto& style = ImGui::GetStyle( );
	//runtime_assert(style.Alpha == fade_.max( ));

	if(!this->Animate( ) && !visible__)
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
		_Get_char_size( ).x * title.chars_count( ) + //string size
		style.FramePadding.x * 2.f +                 //space between and after
		style.WindowBorderSize;

	nstd::memory_backup<float> min_size_backup;
	(void)min_size_backup;

	if(min_size > style.WindowMinSize.x)
		min_size_backup = {style.WindowMinSize.x, min_size};

	return ImGui::Begin(title, nullptr, flags);
}

void window::end( )
{
	if(ignore_end__)
	{
		ignore_end__ = false;
#ifdef CHEAT_GUI_WIDGETS_FADE_CONTENT
		runtime_assert(!fade_alpha_backup_.has_value( ));
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
	fade_.set(uint8_t(-1));
}

void window::toggle( )
{
	visible__ = !visible__;
	fade_.set(visible__ ? 1 : uint8_t(-1));
}

bool window::visible( ) const
{
	return visible__ || animating( );
}

bool window::active( ) const
{
	return visible__ && fade_.done(1);
}

child_window::child_window(animator&& fade)
	: content_background_fader(std::move(fade))
{
}

bool child_window::Begin_impl(ImGuiID id, const ImVec2& size_arg, bool border, ImGuiWindowFlags extra_flags)
{
	return ImGui::BeginChild(id, size_arg, border, extra_flags);
}

bool child_window::begin(const ImVec2& size, bool border, ImGuiWindowFlags_ flags)
{
	return Begin_impl(reinterpret_cast<ImGuiID>(this), size, border, flags);
}

void child_window::end( )
{
	ImGui::EndChild( );
#ifdef CHEAT_GUI_WIDGETS_FADE_CONTENT
	fade_alpha_backup_.restore( );
#endif
}

void child_window::show( )
{
	return fade_.set(1);
}

child_frame_window::child_frame_window(animator&& fade)
	: child_window(std::move(fade))
{
}

bool child_frame_window::Begin_impl(ImGuiID id, const ImVec2& size_arg, bool border, ImGuiWindowFlags extra_flags)
{
	const auto& style = ImGui::GetStyle( );
	[[maybe_unused]]
	const auto backups = std::make_tuple(push_style_color(ImGuiCol_ChildBg, style.Colors[ImGuiCol_FrameBg]),
		push_style_var(ImGuiStyleVar_ChildRounding, style.FrameRounding),
		push_style_var(ImGuiStyleVar_ChildBorderSize, style.FrameBorderSize),
		push_style_var(ImGuiStyleVar_WindowPadding, style.FramePadding));

	return child_window::Begin_impl(id, size_arg, border, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysUseWindowPadding | extra_flags);
}
#endif

#include "cheat/core/console.h"
#include "cheat/gui/tools/cached_text.h"

#include <nstd/enum_tools.h>
#include <nstd/runtime assert.h>
#include <nstd/unistring.h>
#include <nstd/smooth_value.h>

#include <imgui_internal.h>

#include <ranges>
#include <vector>

using namespace cheat::gui;
using namespace widgets;
using namespace tools;

end_token::end_token()
{
	value_ = static_cast<uint8_t>(-1);
}

end_token::end_token(end_token&& other) noexcept
{
	*this = std::move(other);
}

end_token& end_token::operator=(end_token&& other) noexcept
{
	value_ = other.release( );
	return *this;
}

void end_token::set(uint8_t val)
{
	value_ = val;
}

uint8_t end_token::release()
{
	const auto ret = value_;
	value_         = static_cast<uint8_t>(-1);
	return ret;
}

bool end_token::unset() const
{
	return value_ != static_cast<uint8_t>(static_cast<bool>(value_));
}

bool end_token::operator!() const
{
	return value_ != 1;
}

bool end_token::operator==(bool val) const
{
	return !unset( ) && static_cast<bool>(value_) == val;
}

bool end_token::operator!=(bool val) const
{
	return !unset( ) && static_cast<bool>(value_) != val;
}

uint8_t end_token::value() const
{
	return value_;
}

//----

window_end_token_ex::window_end_token_ex()
{
	global_alpha_backup_ = ImGui::GetStyle( ).Alpha;
}

window_end_token_ex::~window_end_token_ex()
{
	switch (value( ))
	{
		case 0:
		case 1:
			ImGui::End( );
			//ImGui::PopFont( );
		case 2:
			global_alpha_backup_.restore( );
			break;
		default:
			global_alpha_backup_.reset( );
			break;
	}
}

//-----

template <class T/*ImGuiWindow*/>
concept imgui_window_has_font_dpi_scale = requires()
{
	typename T::FontDpiScale;
};

static float _Min(float a, float b)
{
	return std::min(a, b);
}

static float _Max(float a, float b)
{
	return std::max(a, b);
}

window_end_token widgets::window2(const window_title& title, bool* open, ImGuiWindowFlags flags)
{
	const auto window_title = get_imgui_string(title.label_legacy);
	const auto window       = ImGui::FindWindowByName(window_title);
	auto& style             = ImGui::GetStyle( );

	const auto backups = [&]
	{
		auto& g          = *ImGui::GetCurrentContext( );
		const auto font  = title.font;
		const auto atlas = g.Font->ContainerAtlas;
		auto& s          = g.DrawListSharedData;;
		s.TexUvLines     = atlas->TexUvLines;
		s.Font           = g.Font;
		s.FontSize       = g.FontSize;

		constexpr auto bck = []<typename ...Ts>(Ts&&...ts)
		{
			return nstd::memory_backup(std::forward<Ts>(ts)...);
		};

		return std::make_tuple(bck(g.Font, font)
							 , bck(g.FontBaseSize, font->FontSize)
							 , bck(g.FontSize)
							 , bck(s.TexUvWhitePixel, atlas->TexUvWhitePixel)
							 , bck(s.TexUvLines, atlas->TexUvLines)
							 , bck(s.Font, font)
							 , bck(s.FontSize, font->FontSize),
							   bck(style.WindowMinSize));
	}( );

	struct title_rect_t
	{
		ImRect layout, clip;
	};

	std::optional<ImRect> title_rect;
	if (!(flags & ImGuiWindowFlags_NoTitleBar) && window
#ifdef IMGUI_HAS_DOCK
		&& !window->DockIsActive
#endif
	)
	{
		constexpr auto font_dpi_scale_assert = []<class T>(T* wnd)
		{
			if constexpr (imgui_window_has_font_dpi_scale<T>)
				runtime_assert(wnd->FontDpiScale == 1, "imgui's dpi scale unsupported");
		};
		font_dpi_scale_assert(window);
		runtime_assert(window->FontWindowScale == 1, "imgui's window font scale unsupported");

		auto pad_l           = style.FramePadding.x;
		auto pad_r           = style.FramePadding.x;
		const auto button_sz = /*title.get_font( )->FontSize*/title.label_size.y;

		const auto has_collapse_button = !(flags & ImGuiWindowFlags_NoCollapse) && style.WindowMenuButtonPosition != ImGuiDir_None;
		const auto has_close_button    = /* window ?*/ window->HasCloseButton /*: open != NULL*/;

		if (has_collapse_button)
		{
			switch (style.WindowMenuButtonPosition)
			{
				case ImGuiDir_Right:
					pad_r += button_sz;
					break;
				case ImGuiDir_Left:
					pad_l += button_sz;
					break;
			}
		}
		if (has_close_button)
			pad_r += button_sz;

		if (pad_l > style.FramePadding.x)
			pad_l += style.ItemInnerSpacing.x;
		if (pad_r > style.FramePadding.x)
			pad_r += style.ItemInnerSpacing.x;

		//warning: watch changes in imgui.cpp
		if (flags & ImGuiWindowFlags_UnsavedDocument)
			pad_l += button_sz * 0.80f;

		const auto title_bar_rect = [&]()-> ImRect
		{
			ImRect rect;
			if (!title.render_manually)
			{
				rect = window->TitleBarRect( );
				rect.Min.x += window->WindowBorderSize;
				rect.Max.x -= window->WindowBorderSize;
			}
			else
			{
				const auto& pos = window->Pos;
				rect.Min        = pos;
				rect.Max.x      = pos.x + std::max(window->SizeFull.x - window->WindowBorderSize
												 , pad_l + pad_r + title.label_size.x + (window->ScrollbarY ? window->ScrollbarSizes.x : 0));
				rect.Max.y = pos.y + window->TitleBarHeight( );
			}

			return rect;
		};

		title_rect = [&]()-> std::optional<ImRect>
		{
			if (!title.render_manually)
				return {};

			const auto tb_rect = title_bar_rect( );

			const auto fix_min_x = [&]
			{
				const auto wpt = static_cast<ImGuiViewportP*>(
					ImGui::
#ifdef IMGUI_HAS_VIEWPORT
				GetWindowViewport
#else
					GetMainViewport
#endif
					( ));

				const auto rect = wpt->GetMainRect( );
				return !rect.Contains(tb_rect);
			}( );

			const auto min_x = !fix_min_x
								   ? tb_rect.Min.x
								   : std::invoke(tb_rect.Min.x > 0 ? _Min : _Max, tb_rect.Min.x, window->InnerRect.Min.x);
			return ImRect(min_x + pad_l
						, std::min(tb_rect.Min.y, window->OuterRectClipped.Min.y)
						, min_x + (window->SizeFull.x - window->WindowBorderSize - pad_r)
						, std::min(tb_rect.Max.y, window->OuterRectClipped.Max.y));
		}( );

		style.WindowMinSize = ImVec2(title.label_size.x + pad_r + pad_l, /*title.label_size.y*/button_sz);
	}

	window_end_token ret;
	ret.set(ImGui::Begin(window_title, open, flags));

	if (title_rect.has_value( ))
	{
		title.render(window->DrawList, title_rect->Min, ImGui::GetColorU32(ImGuiCol_Text), style.WindowTitleAlign, *title_rect, false, true);
	}

	return ret;
}

void window_title::on_update(update_flags flags)
{
	using namespace nstd::enum_operators;

	if (!(flags & update_flags::LABEL_CHANGED))
		return;

	label_legacy    = label;
	render_manually = label_legacy.size( ) != label.size( );

	if (render_manually)
	{
		auto num = "##" + std::to_string(reinterpret_cast<uintptr_t>(this));
		label_legacy.assign(num.begin( ), num.end( ));
	}
}

bool window_wrapped::visible() const
{
	return show || updating( );
}

bool window_wrapped::updating() const
{
	using state = nstd::smooth_object_base::state;
	switch (show_anim.get_state( ))
	{
			//case state::RESTARTED_DELAYED:
			//case state::RESTARTED:
		case state::STARTED:
		case state::RUNNING:
			return true;
		default:
			return false;
	}
}

window_end_token_ex window_wrapped::operator()(bool close_button)
{
	runtime_assert(show_anim.get_target( )->get_value( ) == 1);
	window_end_token_ex token;

	show_anim.update_end(show);
	const auto updated = show_anim.update( );

	if (visible( ))
	{
		token.set(window2(this->title, close_button ? std::addressof(show) : nullptr, flags | temp_flags_).release( ));

		//todo: toggle only of window focused?
		//todo2: ignore toggle when text input active
		/*if (ImGui::IsWindowFocused(ImGuiFocusedFlags_DockHierarchy | ImGuiFocusedFlags_RootWindow))
			temp_flags_ &= ~ImGuiWindowFlags_NoFocusOnAppearing;
		else
			temp_flags_ |= ImGuiWindowFlags_NoFocusOnAppearing;*/
	}
	else if (updated)
	{
		token.set(2);
	}

	return token;
}
