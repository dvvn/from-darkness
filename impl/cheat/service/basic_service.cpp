module;

#include "includes.h"
#include <cppcoro/when_all.hpp>

module cheat.service:basic;

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

size_t basic_service::_Add_dependency(value_type && srv)
{
#ifdef _DEBUG
	auto deps = _Deps<true>( );
	if (std::ranges::find(deps, srv->type( ), &basic_service::type) != deps.end( ))
		runtime_assert("Service already stored!");
#endif 
	deps_.push_back(std::move(srv));
	return deps_.size( ) - 1;
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

		this->load_async( );

		if (!deps_.empty( ))
		{
			using std::views::transform;
			using std::ranges::all_of;

			set_state(service_state::waiting);
			co_await ex.schedule( );
			const auto unwrapped = deps_ | transform([]<typename T>(const T & srv)-> basic_service& { return *srv; });
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