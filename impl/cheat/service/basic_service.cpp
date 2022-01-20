module;

#include "basic_includes.h"

#include <cppcoro/when_all.hpp>
#include <cppcoro/sync_wait.hpp>

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

auto basic_service::start(executor & ex) noexcept -> task_type
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

		this->construct( );

		if (!deps_.empty( ))
		{
			this->set_state(service_state::waiting);
			auto refs = _Deps<true>( );

			std::vector<task_type> tasks;
			tasks.reserve(refs.size( ));
			co_await ex.schedule( );
			for (auto& srv : refs)
				tasks.push_back(srv.start(ex));

			//todo: stop when error detected
			auto results = co_await cppcoro::when_all(std::move(tasks));
			if (std::ranges::any_of(results, [](bool val) { return !val; }))
			{
				runtime_assert("Unable to load other services!");
				this->set_state(service_state::error);
				co_return false;
			}
		}

		this->set_state(service_state::loading);
		co_await ex.schedule( );
		if (!/*co_await*/ this->load( ))
		{
			runtime_assert("Unable to load service!");
			set_state(service_state::error);
			co_return false;
		}

		this->set_state(service_state::loaded);
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
auto basic_service::start(executor & ex, sync_start) noexcept -> task_type::value_type
{
	return cppcoro::sync_wait(start(ex));
}
auto basic_service::start( ) noexcept -> task_type::value_type
{
	executor ex;
	return start(ex, sync_start( ));
}