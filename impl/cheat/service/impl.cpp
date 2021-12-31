module;

#include "includes.h"
#include <nstd/ranges.h>
#include <cppcoro/when_all.hpp>

module cheat.service:core;

using namespace cheat;

basic_service::basic_service( ) = default;
basic_service::~basic_service( ) = default;

basic_service::basic_service(basic_service && other) noexcept
{
	*this = std::move(other);
}

basic_service& basic_service::operator=(basic_service && other) noexcept
{
	std::swap(deps_, other.deps_);
	std::swap(state_, other.state_);

	return *this;
}

auto basic_service::find(const std::type_info & info) const -> const value_type*
{
	return std::_Const_cast(this)->find(info);
}

auto basic_service::find(const std::type_info & info) -> value_type*
{
	for (auto& d : deps_)
	{
		auto& info1 = d->type( );
		if (info == info1)
			return std::addressof(d);
	}
	return nullptr;
}

// ReSharper disable once CppMemberFunctionMayBeConst
void basic_service::erase(const std::type_info & info)
{
	auto& deps = deps_;
	const auto found = std::ranges::find(deps, info, &basic_service::type);
	if (found != deps.end( ))
		deps.erase(found);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void basic_service::erase(const erase_pred & fn)
{
	auto& deps = deps_;
	const auto found = std::ranges::find_if(deps, std::_Pass_fn(fn));
	if (found != deps.end( ))
		deps.erase(found);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void basic_service::erase_all(const erase_pred & fn)
{
	const auto found = std::ranges::remove_if(deps_, std::_Pass_fn(fn));
	/*if (found != deps_.end( ))
		deps_.erase(found, deps_.end( ));*/
	if (!found.empty( ))
		deps_.erase(found.begin( ), found.end( ));
}

// ReSharper disable once CppMemberFunctionMayBeConst
void basic_service::add_dependency(value_type && srv)
{
	runtime_assert(!find(srv->type( )), "Service already stored!");
	deps_.push_back(std::move(srv));
}

service_state basic_service::state( ) const
{
	return state_;
}

auto basic_service::load(executor & ex) noexcept -> load_result
{
	switch (state_)
	{
	case service_state::unset:
	{
		const auto mtx = co_await lock_.scoped_lock_async( );
		//check is other thread do all the work
		switch (state_)
		{
		case service_state::loaded:
			co_return true;
		case service_state::error:
			co_return false;
		case service_state::unset:
			break;
		default:
			runtime_assert("Preload: mutex released, but state isn't sets correctly!");
			std::terminate( );
		}

		if (!deps_.empty( ))
		{
			using std::views::transform;
			using std::ranges::all_of;

			set_state(service_state::waiting);
			co_await ex.schedule( );
			const auto unwrapped = deps_ | transform([]<typename T>(const T& srv)-> basic_service& { return *srv; });
			auto tasks_view = unwrapped | transform([&](basic_service& srv)-> load_result { return srv.load(ex); });
			//todo: stop when error detected
			auto results = co_await when_all(std::vector(tasks_view.begin( ), tasks_view.end( )));
			if (!all_of(results, [](bool val) { return val == true; }))
			{
				runtime_assert("Unable to load other services!");
				set_state(service_state::error);
				co_return false;
			}
		}

		set_state(service_state::loading);
		co_await ex.schedule( );
		if (!/*co_await*/ this->load_impl( ))
		{
			runtime_assert("Unable to load service!");
			set_state(service_state::error);
			co_return false;
		}

		set_state(service_state::loaded);
		co_return true;
	}
	case service_state::waiting:
	case service_state::loading:
	{
		//some other thread loads this service,
		const auto mtx = co_await lock_.scoped_lock_async( );
		switch (state_)
		{
		case service_state::loaded:
			co_return true;
		case service_state::error:
			co_return false;
		default:
			runtime_assert("Postload: mutex released, but state isn't sets correctly!");
			std::terminate( );
		}
	}
	case service_state::loaded:
	{
		co_return true;
	}
	case service_state::error:
	{
		runtime_assert("Service loaded with errors!");
		co_return false;
	}
	default:
	{
		runtime_assert("Unknown state");
		co_return false;
	}
	}
}
