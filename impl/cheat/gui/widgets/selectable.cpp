#include "selectable.h"

#include "cheat/gui/tools/push style color.h"

using namespace cheat;
using namespace gui::widgets;
using namespace gui::tools;
using namespace utl;

//ImGui::PushStyleColor(color_idx, !anim_updated ? header_color : ImVec4(header_color.x, header_color.y, header_color.z, header_color.w * anim__.value( )));

selectable::selectable(bool selected)
{
	if (selected)
		anim__.set(1);
	else
	{
		anim__.set(-1);
		anim__.finish( );
	}
}

bool selectable::operator()(string_wrapper::value_type label, ImGuiSelectableFlags flags, const ImVec2& size)
{
#if 0
	const auto selectable = [&](bool selected)
	{
		return ImGui::Selectable(label, selected, flags, size);
	};

	if(!anim__.update( ))
	{
		if(selectable(selected( )))
		{
			/*select( );
			if (selected_before.has_value( ))
			{
				auto& ptr = selected_before.get( );
				ptr->deselect( );
				ptr = this;
			}*/
			return true;
		}
	}
	else
	{
		const auto  color_index = ImGuiCol_Header;
		const auto& header_color = ImGui::GetStyle( ).Colors[color_index];
		const auto  color = ImVec4(header_color.x, header_color.y, header_color.z, header_color.w * anim__.value( ));

		//@note: if return false here only two object can be animated at same time

		ImGui::PushStyleColor(color_index, color);
		/*auto ret=*/
		(void)selectable(true);
		//return ret;
	}

	return false;

#endif

	return ImGui::Selectable(label,
							 anim__.update( ) ? push_style_color(ImGuiCol_Header, anim__.value( )).val(true) : selected( ),
							 flags, size)
		   /*&& !selected( )*/ && !anim__.updating( );
}

bool selectable::operator()(const string_wrapper& label, /*optional<selectable*&> selected_before,*/ ImGuiSelectableFlags flags, const ImVec2& size)
{
	return invoke(*this, label.imgui( ), flags, size);
}

void selectable::select( )
{
	anim__.set(1);
}

void selectable::deselect( )
{
	anim__.set(-1);
}

void selectable::toggle( )
{
	anim__.set(selected( ) ? -1 : 1);
}

bool selectable::selected( ) const
{
	return anim__.dir( ) == 1;
}

bool selectable::animating( ) const
{
	return anim__.updating( );
}

selectable_base::selectable_base(bool selected): selectable(selected)
{
}

bool selectable_base::operator()(ImGuiSelectableFlags flags, const ImVec2& size)
{
	return invoke(*static_cast<selectable*>(this), Name( ), flags, size);
}
