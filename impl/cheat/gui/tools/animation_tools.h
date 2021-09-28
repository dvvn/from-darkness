#pragma once

#include "button_info.h"

#include <nstd/smooth_value_fwd.h>

struct ImVec4;
using ImU32 = unsigned int;

namespace std
{
	ImVec4 lerp(const ImVec4& a, const ImVec4& b, float val);
}

namespace cheat::gui::tools
{
	class animation_color_helper
	{
		nstd::smooth_value_base<ImVec4>* animation_;

	public:
		animation_color_helper(nstd::smooth_value_base<ImVec4>* animation);

		ImU32 operator()(const ImVec4& clr) const;
	};

	ImU32 get_button_color(button_state state, bool idle_visible
						 , const ImVec4& idle_clr, const ImVec4& hovered_clr, const ImVec4& held_clr, const ImVec4& pressed_clr
						 , nstd::smooth_value_base<ImVec4>* animation);

	ImU32 get_boolean_color(bool value
						  , const ImVec4& clr
						  , nstd::smooth_value_base<ImVec4>* animation);
}
