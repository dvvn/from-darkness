#include "renderable object.h"

#include <array>
#include <imgui_internal.h>

using cheat::gui::objects::renderable;

template <size_t ...I>
static ImGuiID _Get_id_ex(const ImGuiID* arr, std::index_sequence<I...>)
{
	return (arr[I] ^ ...);
}

ImGuiID renderable::get_id( ) const
{
	static_assert(sizeof(ImGuiID) % sizeof(void*) == 0);
	const auto ptr  = this;
	const auto pptr = std::addressof(ptr);
	return _Get_id_ex(reinterpret_cast<const ImGuiID*>(pptr), std::make_index_sequence<sizeof(void*) / sizeof(ImGuiID)>( ));
}

ImGuiID renderable::get_id(ImGuiWindow* wnd) const
{
	const auto seed = wnd->IDStack.back( );
	const auto id   = this->get_id( ) ^ seed;
	ImGui::KeepAliveID(id);
	return id;
}
