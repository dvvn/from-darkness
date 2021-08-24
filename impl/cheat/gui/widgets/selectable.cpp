#include "selectable.h"

#include "cheat/gui/tools/push style color.h"

using namespace cheat;
using namespace gui::widgets;
using namespace gui::tools;

//ImGui::PushStyleColor(color_idx, !anim_updated ? header_color : ImVec4(header_color.x, header_color.y, header_color.z, header_color.w * anim__.value( )));

selectable::selectable(bool selected): selectable_base(selected)
{
}

bool selectable::operator()(prefect_string&& label, ImGuiSelectableFlags_ flags, const ImVec2& size)
{
	const auto selectable = [&](bool selected)
	{
		return ImGui::Selectable((label), selected, flags, size);
	};

	if (!this->Animate( ))
	{
		return selectable(this->selected( ));
	}
	else
	{
		const auto pop = push_style_color(ImGuiCol_Header, this->Anim_value( ));
		(void)pop;
		(void)selectable(true);
		return false;
	}
}

selectable_internal::selectable_internal(bool selected): selectable(selected)
{
}

bool selectable_internal::operator()(ImGuiSelectableFlags_ flags, const ImVec2& size)
{
	return std::invoke(*static_cast<selectable*>(this), Label( ), flags, size);
}
