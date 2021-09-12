#include "renderable object.h"

#include <imgui_internal.h>

using cheat::gui::objects::renderable;

ImGuiID renderable::get_id( ) const
{
	return reinterpret_cast<ImGuiID>(this);
}

ImGuiID renderable::get_id(ImGuiWindow* wnd) const
{
	const auto seed = wnd->IDStack.back( );
	const auto id   = this->get_id( ) ^ seed;
	ImGui::KeepAliveID(id);
	return id;
}
