#include "animated selectable.h"

#include "push style color.h"

using namespace cheat;
using namespace gui::imgui;
using namespace utl;

//ImGui::PushStyleColor(color_idx, !anim_updated ? header_color : ImVec4(header_color.x, header_color.y, header_color.z, header_color.w * anim__.value( )));

animated_selectable::animated_selectable(bool selected)
{
	if (selected)
		anim__.set(1);
	else
	{
		anim__.set(-1);
		anim__.finish( );
	}
}

bool animated_selectable::operator()(string_wrapper::value_type label, /*optional<animated_selectable*&> selected_before,*/ ImGuiSelectableFlags flags, const ImVec2& size)
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

void animated_selectable::select( )
{
	anim__.set(1);
}

void animated_selectable::deselect( )
{
	anim__.set(-1);
}

void animated_selectable::toggle( )
{
	anim__.set(selected( ) ? -1 : 1);
}

bool animated_selectable::selected( ) const
{
	return anim__.dir( ) == 1;
}

bool animated_selectable::animating( ) const
{
	return anim__.updating( );
}

animated_selectable_base::animated_selectable_base(bool selected): animated_selectable(selected)
{
}

bool animated_selectable_base::operator()(ImGuiSelectableFlags flags, const ImVec2& size)
{
	return animated_selectable::operator()(Name( ), flags, size);
}
