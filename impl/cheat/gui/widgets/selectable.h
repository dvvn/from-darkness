#pragma once

#include "selectable base.h"

#include "cheat/gui/tools/string wrapper.h"

#include <imgui.h>

namespace cheat::gui::widgets
{
	class selectable: public selectable_base
	{
	public:
		selectable(bool selected = false);

		bool operator()(tools::perfect_string&& label,
						ImGuiSelectableFlags_   flags = ImGuiSelectableFlags_None, const ImVec2& size = ImVec2(0, 0));
	};

	class selectable_internal: public selectable
	{
		using selectable::operator();

	protected:
		selectable_internal(bool selected = false);

		virtual tools::string_wrapper::value_type Label( ) const = 0;

	public:
		bool operator()(ImGuiSelectableFlags_ flags = ImGuiSelectableFlags_None, const ImVec2& size = ImVec2(0, 0));
	};
}
