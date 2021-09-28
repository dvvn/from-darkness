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

#include "nstd/runtime assert.h"
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
	const auto ret = (value_);
	value_         = static_cast<uint8_t>(-1);
	return ret;
}

bool end_token::unset() const
{
	return value_ != static_cast<bool>(value_);
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
	global_alpha_ = ImGui::GetStyle( ).Alpha;
}

window_end_token_ex::window_end_token_ex(window_end_token_ex&& other) noexcept
{
	*this = std::move(other);
}

window_end_token_ex& window_end_token_ex::operator=(window_end_token_ex&& other) noexcept
{
	end_token::operator=(std::move(other));
	global_alpha_ = other.global_alpha_;
	return *this;
}

window_end_token_ex::~window_end_token_ex()
{
	switch (value( ))
	{
		case 0:
		case 1:
			ImGui::End( );
		case 2:
			ImGui::GetStyle( ).Alpha = global_alpha_;
			break;
	}
}

//-----

class legacy_window_names
{
public:
	using string_type = nstd::unistring<char>;
	using value_type = std::pair<size_t, string_type>;

	auto operator[](const cached_text& title)
	{
		auto hash = title.get_label_hash( );

		string_type* str;
		// ReSharper disable once CppTooWideScopeInitStatement
		const auto found = std::ranges::find(names_, title.get_label_hash( ), &value_type::first);
		if (found != names_.end( ))
		{
			str = std::addressof(found->second);
		}
		else
		{
			auto temp_str = u8"##" + string_type(
#if 0
				title.get_label( )._Uni_unwrap( )
#else
									std::to_string(reinterpret_cast<uintptr_t>(std::addressof(title)))
#endif
									);
			names_.push_back(std::make_pair(hash, std::move(temp_str)));
			str = std::addressof(names_.back( ).second);
		}

		return get_imgui_string(*str);
	}

private:
	std::vector<value_type> names_;
};

static legacy_window_names _Window_names;

window_end_token widgets::window2(const cached_text& title, bool* open, ImGuiWindowFlags flags)
{
	//ImGui::PushFont(title.get_font( ));

	window_end_token ret;
	ret.set(ImGui::Begin(_Window_names[title], open, flags));

	const auto window = ImGui::GetCurrentWindowRead( );

	if (false && !(flags & ImGuiWindowFlags_NoTitleBar))
	{
		runtime_assert(window->FontDpiScale == 1, "imgui's dpi scale unsupported");
		runtime_assert(window->FontWindowScale == 1, "imgui's window font scale unsupported");

		const auto& style = ImGui::GetStyle( );

		const auto title_bar_rect = [&]()-> ImRect
		{
			const auto rect = window->TitleBarRect( );
			return {
					rect.Min.x + window->WindowBorderSize
				  , rect.Min.y
				  , rect.Max.x - window->WindowBorderSize
				  , rect.Max.y
			};
		}( );

		auto pad_l           = style.FramePadding.x;
		auto pad_r           = style.FramePadding.x;
		const auto button_sz = title.get_font( )->FontSize;

		const auto has_collapse_button = !(flags & ImGuiWindowFlags_NoCollapse) && (style.WindowMenuButtonPosition != ImGuiDir_None);
		const auto has_close_button    = /*(p_open != NULL)*/window->HasCloseButton;

		if (has_collapse_button)
		{
			switch (static_cast<ImGuiDir_>(style.WindowMenuButtonPosition))
			{
				case ImGuiDir_Left:
					pad_r += button_sz;
					break;
				case ImGuiDir_Right:
					pad_l += button_sz;
					break;
				default:
					break;
			}
		}
		if (has_close_button)
			pad_r += button_sz;

		if (pad_l > style.FramePadding.x)
			pad_l += style.ItemInnerSpacing.x;
		if (has_close_button)
			pad_r += style.ItemInnerSpacing.x;

		if (style.WindowTitleAlign.x > 0.0f && style.WindowTitleAlign.x < 1.0f)
		{
			const auto centerness = ImSaturate(1.0f - ImFabs(style.WindowTitleAlign.x - 0.5f) * 2.0f); // 0.0f on either edges, 1.0f on center
			const auto pad_extend = ImMin(ImMax(pad_l, pad_r), title_bar_rect.GetWidth( ) - (pad_l + pad_r + title.get_label_size( ).x));
			const auto pad_max    = pad_extend * centerness;

			pad_l = ImMax(pad_l, pad_max);
			pad_r = ImMax(pad_r, pad_max);
		}
		/*ImRect layout_r(title_bar_rect.Min.x + pad_l, title_bar_rect.Min.y, title_bar_rect.Max.x - pad_r, title_bar_rect.Max.y);
		ImRect clip_r(layout_r.Min.x, layout_r.Min.y, ImMin(layout_r.Max.x + g.Style.ItemInnerSpacing.x, title_bar_rect.Max.x), layout_r.Max.y);*/
		const auto layout_r = ImVec4(title_bar_rect.Min.x + pad_l,
									 title_bar_rect.Min.y,
									 ImMin(title_bar_rect.Max.x - pad_r + style.ItemInnerSpacing.x, title_bar_rect.Max.x),
									 title_bar_rect.Max.y);
		title.render(window->DrawList, {layout_r.x, layout_r.y}, ImGui::GetColorU32(ImGuiCol_Text), std::addressof(layout_r));
	}

	//ImGui::PopFont( );

	return (ret);
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
