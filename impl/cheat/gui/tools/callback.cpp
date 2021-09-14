// ReSharper disable CppMemberFunctionMayBeConst
#include "callback.h"

#include <imgui.h>

#include "cheat/gui/objects/renderable object.h"

// ReSharper disable once CppUnusedIncludeDirective
#include "nstd/runtime assert.h"

#include <string>
#include <veque.hpp>

#include <algorithm>
#include <functional>
#include <optional>
#include <ranges>

using namespace cheat::gui::tools;

callback_state::callback_state( )
{
	start = ImGui::GetTime( );
}

void callback_state::tick( )
{
	++ticks;
	duration += ImGui::GetIO( ).DeltaTime/*ImGui::GetTime( ) - start*/;
}

callback_data::callback_data(objects::renderable* caller, ImGuiID id)
	: caller(caller), id(id)
{
}

callback_data::callback_data(objects::renderable* caller)
	: callback_data(caller, caller->get_id( ))
{
}

callback_info::callback_info(callback_func_type&& func, bool repeat)
	: fn(std::make_unique<callback_func_type>(std::move(func)))
	, repeat(repeat)
{
}

callback_info::~callback_info( )                                  = default;
callback_info::callback_info(callback_info&&) noexcept            = default;
callback_info& callback_info::operator=(callback_info&&) noexcept = default;

struct callback::impl
{
	veque::veque<callback_info>   storage;
	std::optional<callback_state> state;
};

callback::callback( )
{
	impl_ = std::make_unique<impl>( );
}

callback::~callback( )                             = default;
callback::callback(callback&&) noexcept            = default;
callback& callback::operator=(callback&&) noexcept = default;

bool callback::active( ) const
{
	return impl_->state.has_value( );
}

void callback::reset( )
{
	auto& [storage, state] = *impl_;
	if (!state.has_value( ))
		return;

	state.reset( );
	for (auto& skip: storage | std::views::transform(&callback_info::skip))
		skip = false;
}

void callback::operator()(const callback_data& data)
{
	auto& [storage, state] = *impl_;

	if (!state.has_value( ))
		state.emplace( );

	if (storage.empty( ))
		return;

	state->tick( );

	for (auto& [fn, repeat, skip]: storage)
	{
		if (skip)
			continue;

		std::invoke(*fn, data, *state);

		if (!repeat)
			skip = true;
	}
}

void callback::add(callback_info&& info)
{
	impl_->storage.push_front(std::move(info));
}

void two_way_callback::add(callback_info&& info, ways way)
{
	runtime_assert(way == WAY_TRUE || way == WAY_FALSE);
	(way == WAY_TRUE ? way_true : way_false).add(std::move(info));
}

void two_way_callback::operator()(bool value, const callback_data& data)
{
	if (value)
	{
		way_false.reset( );
		way_true(data);
	}
	else if (way_true.active( ) || way_false.active( ))
	{
		way_true.reset( );
		way_false(data);
	}
}
