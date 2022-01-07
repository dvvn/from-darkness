#pragma once

#include <imgui.h>

#include <variant>

import nstd.mem;

namespace cheat::gui::tools
{
	//PushStyleVar(ImGuiCol idx, const ImVec4& col)
	class push_style_var
	{
	public:
		push_style_var( );
		push_style_var(ImGuiStyleVar idx, float val);
		push_style_var(ImGuiStyleVar idx, const ImVec2& val);

	private:
		std::variant<nstd::mem::backup<float>, nstd::mem::backup<ImVec2>> data_;
	};
}
