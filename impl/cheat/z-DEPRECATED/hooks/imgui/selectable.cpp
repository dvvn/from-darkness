#include "selectable.h"

#include "button behavior.h"
#include "get color u32.h"

using namespace cheat;
using namespace hooks;
using namespace imgui;
using namespace utl;

selectable::selectable( )
{
	this->Wait_for<button_behavior>( );
}

auto selectable::Init( ) -> void
{
	target_func_ = method_info::make_static(detail::selectable_fn);

	this->hook( );
	this->enable( );
}

auto selectable::Callback(const char* label, bool selected, ImGuiSelectableFlags flags, const ImVec2& size_arg) -> void
{
	auto& flags_ex = reinterpret_cast<bitflag<ImGuiSelectableFlags_>&>(flags);
	if (flags_ex.has(ImGuiSelectableFlags_Disabled))
		return;

	const auto id = ImGui::GetCurrentWindowRead( )->GetID(label);

	auto [selected_data,selected_data_created] = selected__.try_emplace(id, selected);
	auto [color_data, color_data_created] = get_color_u32::get_ptr( )->get_color(id, ImGuiCol_Header);

	if (color_data_created)
		return;
	auto& [anim, color, skip,inverse_colors,self_alpha] = color_data;

	auto&      selected_before = selected_data.value( );
	const auto deselected = color_data_created ? false : selected_before && !selected;
	const auto selected_first_time = color_data_created ? false : !selected_before && selected;
	selected_before = selected;

	auto button_callback = [&](bool pressed, const ImRect& bb, ImGuiID, bool& hovered, bool& held, bitflag<ImGuiButtonFlags_>)
	{
		if (held || pressed)
			return;

		/*if (color.from != ImGuiCol_Header && color_to != ImGuiCol_Header)
			return;*/
		skip = true;

		ImU32 bg_col;
		if ((deselected || selected_first_time) && !hovered)
		{
			//bg_col = get_color_u32::get( ).call_original(ImGuiCol_Header, 0.f);
			color.from = /*ImGuiCol_WindowBg*/ImGuiCol_Header;
			color.to = ImGuiCol_Header;
			self_alpha = 1;
			anim.set(deselected ? -1 : 1);
			auto col=ImGui::GetStyleColorVec4(ImGuiCol_Header);
			if (anim.update( ))
			{
				col.w*=anim.value();
			}
			bg_col=ImGui::ColorConvertFloat4ToU32(col);
		}
		else
		{
			const auto color_to = (held && hovered) ? ImGuiCol_HeaderActive : hovered ? ImGuiCol_HeaderHovered : ImGuiCol_Header;
			bg_col = ImGui::GetColorU32(color_to);
		}

		if (anim.updating( ))
		{
			hovered = false;
			selected = false;
			held = false;
			flags_ex.remove(ImGuiSelectableFlags_DrawHoveredWhenHeld);
			ImGui::RenderFrame(bb.Min, bb.Max, bg_col, false);
		}
	};

	// window->DC.LastItemRect /// LastItemDisplayRect

	button_behavior::get_ptr( )->set_func(id, button_callback);
	this->call_original_ex(label, selected, flags, size_arg);
	button_behavior::get_ptr( )->reset_func( );
	skip = false;
}
