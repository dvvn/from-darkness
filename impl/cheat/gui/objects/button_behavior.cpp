// ReSharper disable CppMemberFunctionMayBeConst
#include "button_behavior.h"

#include <imgui_internal.h>

using namespace cheat::gui::objects;

struct button_behavior::impl
{
	tools::two_way_callback on_press, on_hovered, on_held;

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

button_behavior::callback_data_ex::callback_data_ex(const callback_data& data)
	: callback_data(data)
{
}

void button_behavior::invoke_button_callbacks(const ImRect& rect, callback_data_ex& data) const
{
	auto& on_press   = impl_->on_press;
	auto& on_hovered = impl_->on_hovered;
	auto& on_held    = impl_->on_held;

	data.pressed = ImGui::ButtonBehavior(rect, data.id, &data.hovered, &data.held, this->get_button_flags( ));

	on_hovered(/*!data.held&&*/data.hovered, data);
	on_held(data.held, data);
	on_press(data.pressed, data);
}

// ReSharper disable CppMemberFunctionMayBeConst

void button_behavior::add_pressed_callback(tools::callback_info&& info, tools::two_way_callback::ways way)
{
	impl_->on_press.add(std::move(info), way);
}

void button_behavior::add_hovered_callback(tools::callback_info&& info, tools::two_way_callback::ways way)
{
	impl_->on_hovered.add(std::move(info), way);
}

void button_behavior::add_held_callback(tools::callback_info&& info, tools::two_way_callback::ways way)
{
	impl_->on_held.add(std::move(info), way);
}
