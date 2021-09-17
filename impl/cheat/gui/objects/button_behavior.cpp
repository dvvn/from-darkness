// ReSharper disable CppMemberFunctionMayBeConst
#include "button_behavior.h"

#include <imgui_internal.h>

using namespace cheat::gui::objects;

struct button_behavior::impl
{
	tools::two_way_callback on_press, on_hovered, on_held;
	button_callback_data    last_callback_data;

	union
	{
		ImGuiButtonFlags_        f = ImGuiButtonFlags_None;
		ImGuiButtonFlagsPrivate_ fp;
		ImGuiButtonFlags         fi;
		//-
	} flags;
};

button_behavior::button_behavior( )
{
	impl_ = std::make_unique<impl>( );
}

button_behavior::~button_behavior( )                                    = default;
button_behavior::button_behavior(button_behavior&&) noexcept            = default;
button_behavior& button_behavior::operator=(button_behavior&&) noexcept = default;

void button_behavior::set_button_flags(ImGuiButtonFlags flags)
{
	impl_->flags.fi = flags;
}

ImGuiButtonFlags button_behavior::get_button_flags( ) const
{
	return impl_->flags.fi;
}

//button_behavior::callback_data_ex::callback_data_ex(const callback_data& data)
//	: callback_data(data)
//{
//}

void button_behavior::invoke_button_callbacks(const ImRect& rect, ImGuiID id) const
{
	auto& on_press   = impl_->on_press;
	auto& on_hovered = impl_->on_hovered;
	auto& on_held    = impl_->on_held;

	// ReSharper disable once CppJoinDeclarationAndAssignment
	bool hovered, held, pressed;

	pressed = ImGui::ButtonBehavior(rect, id, std::addressof(hovered), std::addressof(held), this->get_button_flags( ));

	on_hovered(/*!held&&*/hovered);
	on_held(held);
	on_press(pressed);

	// ReSharper disable once CppUseStructuredBinding
	auto& data = impl_->last_callback_data;
	//--
	data.held    = held;
	data.hovered = hovered;
	data.pressed = pressed;
	data.id      = id;
}

button_behavior::button_callback_data button_behavior::get_last_callback_data( ) const
{
	return impl_->last_callback_data;
}

// ReSharper disable CppMemberFunctionMayBeConst

void button_behavior::add_pressed_callback(tools::callback_info&& info, tools::two_way_callback::ways way)
{
	impl_->on_press[way].add(std::move(info));
}

void button_behavior::add_hovered_callback(tools::callback_info&& info, tools::two_way_callback::ways way)
{
	impl_->on_hovered[way].add(std::move(info));
}

void button_behavior::add_held_callback(tools::callback_info&& info, tools::two_way_callback::ways way)
{
	impl_->on_held[way].add(std::move(info));
}

#ifdef CHEAT_GUI_CALLBACK_HAVE_INDEX
  bool button_behavior::erase_pressed_callback(const tools::callback_id& ids, tools::two_way_callback::ways way)
{
	return impl_->on_press[way].erase(ids);
}

bool button_behavior::erase_hovered_callback(const tools::callback_id& ids, tools::two_way_callback::ways way)
{
	return impl_->on_hovered[way].erase(ids);
}

bool button_behavior::erase_held_callback(const tools::callback_id& ids, tools::two_way_callback::ways way)
{
	return impl_->on_held[way].erase(ids);
}
#endif
