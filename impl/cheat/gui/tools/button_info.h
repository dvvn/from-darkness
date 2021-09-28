#pragma once

#include <cstdint>

struct ImRect;
using ImGuiID = unsigned int;
using ImGuiButtonFlags = int;

namespace cheat::gui::tools
{
	//_ACTIVE => already set and continues
	enum class button_state :int8_t
	{
		UNKNOWN = -1
	  , IDLE
	  , INACTIVE
	  , HOVERED
	  , HOVERED_ACTIVE
	  , HELD
	  , HELD_ACTIVE
	  , PRESSED
	};

	button_state button_behavior(const ImRect& bb, ImGuiID id, bool unreachable = false, ImGuiButtonFlags flags = 0);
}
