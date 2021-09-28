#pragma once
#include "cheat/gui/tools/button_info.h"

#include <nstd/smooth_value_fwd.h>

struct ImVec4;

namespace cheat::gui::tools
{
	class cached_text;
}

namespace cheat::gui::widgets
{
	tools::button_state checkbox2(const tools::cached_text& label, bool& value
								, nstd::smooth_value_base<ImVec4>* bg_animation    = nullptr
								, nstd::smooth_value_base<ImVec4>* check_animation = nullptr);
}
