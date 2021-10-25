#pragma once

#include "nstd/memory backup.h"

#include <imgui.h>

#include <variant>

namespace cheat::gui::tools
{
	//PushStyleVar(ImGuiCol idx, const ImVec4& col)
	class push_style_var
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

	private:
		template <typename T>
		void emplace(T& backup, T new_value)
		{
			data_.emplace<nstd::memory_backup<T>>(nstd::memory_backup<T>(backup, new_value));
		}

		std::variant<nstd::memory_backup<float>, nstd::memory_backup<ImVec2>> data_;
	};
}
