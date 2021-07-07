#pragma once

namespace cheat::gui::imgui
{
	//PushStyleVar(ImGuiCol idx, const ImVec4& col)
	class push_style_var: utl::variant<utl::mem::memory_backup<float>, utl::mem::memory_backup<ImVec2>>
	{
	public:
		push_style_var( );
		push_style_var(ImGuiStyleVar idx, float val);
		push_style_var(ImGuiStyleVar idx, const ImVec2& val);

		template <typename T1>
		_NODISCARD T1 val(T1&& val)
		{
			(void)this;
			return val;
		}
	};
}
