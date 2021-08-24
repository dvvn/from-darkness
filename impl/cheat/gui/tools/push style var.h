#pragma once

namespace cheat::gui::tools
{
	//PushStyleVar(ImGuiCol idx, const ImVec4& col)
	class push_style_var: std::variant<nstd::memory_backup<float>, nstd::memory_backup<ImVec2>>
	{
	public:
		push_style_var( );
		push_style_var(ImGuiStyleVar idx, float val);
		push_style_var(ImGuiStyleVar idx, const ImVec2& val);

		template <typename T>
		_NODISCARD decltype(auto) val(T&& val)
		{
			(void)this;
			return std::forward<T>(val);
		}
	};
}
