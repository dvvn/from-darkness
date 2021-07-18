#include "selectable.h"

#include "cheat/gui/tools/push style color.h"

using namespace cheat;
using namespace gui::widgets;
using namespace gui::tools;
using namespace utl;

//ImGui::PushStyleColor(color_idx, !anim_updated ? header_color : ImVec4(header_color.x, header_color.y, header_color.z, header_color.w * anim__.value( )));

selectable::selectable(bool selected): selectable_base(selected)
{
}

bool selectable::operator()(string_wrapper::value_type label, ImGuiSelectableFlags_ flags, const ImVec2& size)
{
	return ImGui::Selectable(label,
							 this->Update( ) ? push_style_color(ImGuiCol_Header, this->Anim_value( )).val(true) : selected( ),
							 flags, size)
		   /*&& !selected( )*/ && !this->animating( );
}

bool selectable::operator()(const string_wrapper& label, ImGuiSelectableFlags_ flags, const ImVec2& size)
{
	return invoke(*this, label.imgui( ), flags, size);
}

selectable_internal::selectable_internal(bool selected): selectable(selected)
{
}

bool selectable_internal::operator()(ImGuiSelectableFlags_ flags, const ImVec2& size)
{
	return invoke(*static_cast<selectable*>(this), Label( ), flags, size);
}
