#pragma once

#include <imgui_internal.h>

#include <cstdint>

namespace cheat::gui::tools
{
	//_ACTIVE => already set and continues
	enum class button_state :int8_t
	{
		/*	OUT = -1
		  , IDLE
		  , IN
		  , REPEAT*/
		UNKNOWN=-1,
		IDLE
	  , INACTIVE
	  , HOVERED
	  , HOVERED_ACTIVE
	  , HELD
	  , HELD_ACTIVE
	  , PRESSED
	};

	union button_flags
	{
		button_flags()
		{
			f = ImGuiButtonFlags_None;
		}

		ImGuiButtonFlags_ f;
		ImGuiButtonFlagsPrivate_ fp;
		ImGuiButtonFlags fi;
	};

	button_state button_behavior(const ImRect& bb, ImGuiID id,bool unreachable=0, button_flags flags = {});
}
