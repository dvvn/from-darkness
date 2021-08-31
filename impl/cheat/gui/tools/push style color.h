#pragma once

#include "nstd/memory backup.h"

#include <imgui.h>

namespace cheat::gui::tools
{
	//PushStyleColor(ImGuiCol idx, const ImVec4& col)
	class push_style_color: public nstd::memory_backup<ImVec4>
	{
	public:
		push_style_color( ) = default;
		push_style_color(ImGuiCol idx, const ImVec4& col);
		push_style_color(ImGuiCol idx, ImU32 col);
		push_style_color(ImGuiCol idx, float alpha_scale);
	};

	//#define IMGUI_PUSH_STYLE_COLOR(idx, data)\
	//	const auto _CONCAT(im_psc,__LINE__) = imgui::push_style_color(idx,data); (void)_CONCAT(im_psc, __LINE__);
}
