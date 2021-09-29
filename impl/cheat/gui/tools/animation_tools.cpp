#include "animation_tools.h"
#include "button_info.h"

#include <nstd/smooth_value.h>

#include <imgui.h>

ImVec4 std::lerp(const ImVec4& a, const ImVec4& b, float val)
{
	return {lerp(a.x, b.x, val), lerp(a.y, b.y, val), lerp(a.z, b.z, val), lerp(a.w, b.w, val)};
}

using nstd::smooth_value_base;
using namespace cheat::gui;
using namespace tools;

template <template <typename> typename T>
static ImVec4 _Color_set_alpha(const ImVec4& color, float val, T<float>&& operation = {})
{
	return {color.x, color.y, color.z, operation(color.w, val)};
}

static ImVec4 _Color_remove_alpha(const ImVec4& color)
{
	return {color.x, color.y, color.z, 0};
}

animation_color_helper::animation_color_helper(smooth_value_base<ImVec4>* animation)
	: animation_(animation)
{
}

ImU32 animation_color_helper::operator()(const ImVec4& clr) const
{
	auto& result = [&]()-> const ImVec4&
	{
		if (!animation_)
			return clr;

		animation_->update_end(clr);
		animation_->update( );
		return animation_->get_target( )->get_value( );
	}( );

	const auto alpha = ImGui::GetStyle( ).Alpha;
	return ImGui::ColorConvertFloat4ToU32(alpha == 1 ? result : _Color_set_alpha<std::multiplies>(result, alpha));
}

//---

ImU32 tools::get_button_color(button_state state, bool idle_visible
							, const ImVec4& idle_clr, const ImVec4& hovered_clr, const ImVec4& held_clr, const ImVec4& pressed_clr
							, smooth_value_base<ImVec4>* animation)
{
	const animation_color_helper get_color = animation;

	switch (state)
	{
		case button_state::IDLE:
		case button_state::INACTIVE:
			return get_color(idle_visible || idle_clr.w == 0 ? idle_clr : _Color_set_alpha<std::multiplies>(idle_clr, 0));
		case button_state::HOVERED:
		case button_state::HOVERED_ACTIVE:
			return get_color(hovered_clr);
		case button_state::HELD:
		case button_state::HELD_ACTIVE:
			return get_color(held_clr);
		case button_state::PRESSED:
			return get_color(pressed_clr);
		default: throw;
	}
}

ImU32 tools::get_boolean_color(bool value, const ImVec4& clr, smooth_value_base<ImVec4>* animation)
{
	const animation_color_helper get_color = animation;
	return get_color(value || clr.w == 0 ? clr : _Color_remove_alpha(clr));
}
