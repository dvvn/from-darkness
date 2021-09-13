#include "button_behavior.h"

#include <imgui_internal.h>

// ReSharper disable once CppUnusedIncludeDirective
#include <string>
#include <veque.hpp>

#include <algorithm>
#include <functional>
#include <optional>
#include <vector>
#include <ranges>

using namespace cheat::gui::objects;

button_behavior::callback_state::callback_state( )
{
	start = ImGui::GetTime( );
}

void button_behavior::callback_state::tick( )
{
	++ticks;
	duration = ImGui::GetTime( ) - start;
}

struct button_behavior::impl
{
	class callback
	{
	public:
		struct info
		{
			callback_type fn;

			bool repeat; //todo: timer
			bool skip = false;
		};

	private:
		veque::veque<info>            storage_;
		std::optional<callback_state> state_;

	public:
		bool active( ) const
		{
			return state_.has_value( );
		}

		void reset( )
		{
			if (!state_.has_value( ))
				return;

			state_.reset( );
			for (auto& skip: storage_ | std::views::transform(&info::skip))
				skip = false;
		}

		void operator()(const callback_data& data)
		{
			if (!state_.has_value( ))
				state_.emplace( );

			if (storage_.empty( ))
				return;

			state_->tick( );

			for (auto& [fn, repeat, skip]: storage_)
			{
				if (skip)
					continue;

				std::invoke(fn, data, *state_);

				if (!repeat)
					skip = true;
			}
		}

		void add(info&& cb)
		{
			storage_.push_front(std::move(cb));
		}
	};

	struct two_way_callback
	{
		callback in, out;

		void operator()(bool value, const callback_data& data)
		{
			if (value)
			{
				out.reset( );
				in(data);
			}
			else if (in.active( ) || out.active( ))
			{
				in.reset( );
				out(data);
			}
		}
	};

	two_way_callback on_press, on_hovered, on_held;
};

button_behavior::button_behavior( )
{
	impl_ = std::make_unique<impl>( );
}

button_behavior::~button_behavior( )                                    = default;
button_behavior::button_behavior(button_behavior&&) noexcept            = default;
button_behavior& button_behavior::operator=(button_behavior&&) noexcept = default;

void button_behavior::invoke_callbacks(const ImRect& rect, callback_data& data) const
{
	data.pressed = ImGui::ButtonBehavior(rect, data.id, &data.hovered, &data.held);

	auto& [on_press, on_hovered, on_held] = *impl_;

	on_hovered(/*!data.held&&*/data.hovered, data);
	on_held(data.held, data);
	on_press(data.pressed, data);
}

// ReSharper disable CppMemberFunctionMayBeConst

void button_behavior::add_pressed_callback(callback_type&& callback, bool repeat)
{
	impl_->on_press.in.add({std::move(callback), repeat});
}

void button_behavior::add_hovered_callback(callback_type&& callback, bool repeat)
{
	impl_->on_hovered.in.add({std::move(callback), repeat});
}

void button_behavior::add_held_callback(callback_type&& callback, bool repeat)
{
	impl_->on_held.in.add({std::move(callback), repeat});
}

void button_behavior::add_unpressed_callback(callback_type&& callback, bool repeat)
{
	impl_->on_press.out.add({std::move(callback), repeat});
}

void button_behavior::add_unhovered_callback(callback_type&& callback, bool repeat)
{
	impl_->on_hovered.out.add({std::move(callback), repeat});
}

void button_behavior::add_unheld_callback(callback_type&& callback, bool repeat)
{
	impl_->on_held.out.add({std::move(callback), repeat});
}
