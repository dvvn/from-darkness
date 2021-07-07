#include "button behavior.h"

#include "get color u32.h"

#include "cheat/gui/user input.h"

using namespace cheat;
using namespace hooks;
using namespace imgui;
using namespace utl;

button_behavior::button_behavior( )
{
	this->Wait_for<gui::user_input>( );
}

auto button_behavior::set_func(ImGuiID id, func_type&& func) -> void
{
	id__.emplace(id);
	func__ = move(func);
}

auto button_behavior::reset_func( ) -> void
{
	id__.reset( );
}

auto button_behavior::Init( ) -> void
{
	target_func_ = method_info::make_static(ImGui::ButtonBehavior);

	this->hook( );
	this->enable( );
}

auto button_behavior::Callback(const ImRect& bb, ImGuiID id, bool* out_hovered, bool* out_held, ImGuiButtonFlags flags) -> void
{
	if (!id__ || id != *id__)
		return;

	const auto ret = this->call_original_ex(bb, id, out_hovered, out_held, flags);
	func__(ret, bb, id, *out_hovered, *out_held, flags);
	id__.reset( );
}
