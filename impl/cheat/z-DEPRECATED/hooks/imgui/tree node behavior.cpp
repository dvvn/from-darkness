#include "tree node behavior.h"

#include "button behavior.h"
#include "get color u32.h"

using namespace cheat;
using namespace hooks;
using namespace imgui;
using namespace utl;

tree_node_behavior::tree_node_behavior( )
{
	this->Wait_for<button_behavior>( );
}

auto tree_node_behavior::Init( ) -> void
{
	target_func_ = method_info::make_static(ImGui::TreeNodeBehavior);

	this->hook( );
	this->enable( );
}

auto tree_node_behavior::Callback(ImGuiID id, ImGuiTreeNodeFlags flags, const char* label, const char* label_end) -> void
{
	auto& flags_ex = reinterpret_cast<bitflag<ImGuiTreeNodeFlags_>&>(flags);
	if (flags_ex.has(ImGuiTreeNodeFlags_Framed, ImGuiTreeNodeFlags_Selected))
		return;

	auto [color_data, color_data_created] = get_color_u32::get_ptr( )->get_color(id, ImGuiCol_Header);

	if (color_data_created)
		return;
	auto &anim=color_data.anim;
	auto &color=color_data.color;
	auto &skip=color_data.skip;

	auto button_callback = [&](bool pressed, const ImRect& bb, ImGuiID, bool& hovered, bool& held, bitflag<ImGuiButtonFlags_>)
	{
		if (held || pressed)
			return;

		const auto& rect = GImGui->CurrentWindow->DC.LastItemDisplayRect;
		const auto  color_to = (held && hovered) ? ImGuiCol_HeaderActive : hovered ? ImGuiCol_HeaderHovered : ImGuiCol_Header;

		/*if (color.from != ImGuiCol_Header && color_to != ImGuiCol_Header)
			return;*/

		const auto bg_col = ImGui::GetColorU32(color_to);
		skip = true;
		if (anim.update( ))
		{
			hovered = false;
			ImGui::RenderFrame(rect.Min, rect.Max, bg_col, false);
		}
	};

	// window->DC.LastItemRect /// LastItemDisplayRect

	button_behavior::get_ptr( )->set_func(id, button_callback);
	this->call_original_ex(id, flags, label, label_end);
	button_behavior::get_ptr( )->reset_func( );
	skip = false;
}
