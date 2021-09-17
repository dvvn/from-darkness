// ReSharper disable CppMemberFunctionMayBeConst
#include "callback.h"

#include <imgui.h>

#include "cheat/gui/objects/renderable object.h"

// ReSharper disable once CppUnusedIncludeDirective
#include "nstd/overload.h"
#include "nstd/runtime assert.h"

#include <string>
#include <veque.hpp>

#include <algorithm>
#include <functional>
#include <optional>
#include <ranges>
#include <variant>
#include <compare>

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

//----

struct callback_fn::impl
{
	using fn_type = std::variant<func_type, func_type2>;

	fn_type fn;
};

callback_fn::callback_fn( )
{
	impl_ = std::make_unique<impl>( );
}

callback_fn::~callback_fn( )                                = default;
callback_fn::callback_fn(callback_fn&&) noexcept            = default;
callback_fn& callback_fn::operator=(callback_fn&&) noexcept = default;

callback_fn::callback_fn(func_type&& fn)
	: callback_fn( )
{
	impl_->fn.emplace<func_type>(std::move(fn));
}

callback_fn::callback_fn(func_type2&& fn)
	: callback_fn( )
{
	impl_->fn.emplace<func_type2>(std::move(fn));
}

void callback_fn::operator()(const callback_state& state) const
{
	std::visit(nstd::overload(
		[&](const func_type& fn) { fn(state); }
	  , [](const func_type2& fn) { fn( ); }), impl_->fn);
}

//----

callback_info::callback_info(callback_fn&& func
#ifdef CHEAT_GUI_CALLBACK_HAVE_INDEX
						   , callback_id id
#endif
						   , bool repeat)
	: fn(std::move(func))
#ifdef CHEAT_GUI_CALLBACK_HAVE_INDEX
	, id(id)
#endif
	, repeat(repeat)
{
#ifdef CHEAT_GUI_CALLBACK_HAVE_INDEX
	runtime_assert(id != -1);
#endif
}

callback_info::~callback_info( )                                  = default;
callback_info::callback_info(callback_info&&) noexcept            = default;
callback_info& callback_info::operator=(callback_info&&) noexcept = default;

struct callback::impl
{
	using storage_type = veque::veque<callback_info>;

	storage_type                  storage;
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

void callback::operator()( )
{
	auto& [storage, state] = *impl_;

	if (storage.empty( ))
		return;

	auto funcs = storage | std::views::filter([](const callback_info& info) { return info.skip == false; });
	if (funcs.empty( ))
		return;

	if (!state.has_value( ))
		state.emplace( );

	state->tick( );

	for (auto& info: funcs)
	{
		info.fn(*state);

		if (!info.repeat)
			info.skip = true;
	}
}

void callback::add(callback_info&& info)
{
	auto& storage = impl_->storage;
#ifdef CHEAT_GUI_CALLBACK_HAVE_INDEX
	runtime_assert(std::ranges::find(storage, info.id, &callback_info::id) == storage.end( ));
#endif
	storage.push_front(std::move(info));
}

#ifdef CHEAT_GUI_CALLBACK_HAVE_INDEX
bool callback::erase(callback_id id)
{
	auto& storage = impl_->storage;
	auto  target = std::ranges::find(storage, id, &callback_info::id);
	if(target == storage.end( ))
		return false;

	storage.erase(target);
	return true;
}
#endif

callback& two_way_callback::operator[](ways way)
{
	runtime_assert(way == WAY_TRUE || way == WAY_FALSE);
	return (way == WAY_TRUE ? way_true : way_false);
}

void two_way_callback::operator()(bool value)
{
	if (value)
	{
		way_false.reset( );
		way_true( );
	}
	else if (way_true.active( ) || way_false.active( ))
	{
		way_true.reset( );
		way_false( );
	}
}
