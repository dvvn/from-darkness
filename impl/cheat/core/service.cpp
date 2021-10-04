#include "service.h"

#include "csgo interfaces.h"

#include <cppcoro/static_thread_pool.hpp>
#include <cppcoro/async_mutex.hpp>

using namespace cheat;

template <typename T>
static void _Loading_access_assert([[maybe_unused]] T&& state)
{
	runtime_assert(state != service_state::loading && state != service_state::waiting, "Unable to modify running service!");
}

service_base::~service_base()
{
	_Loading_access_assert(state_);
}

service_base::service_base(service_base&& other) noexcept
{
	*this = std::move(other);
}

service_base& service_base::operator=(service_base&& other) noexcept
{
	_Loading_access_assert(this->state_);
	_Loading_access_assert(other.state_);

	std::swap(state_, other.state_);
	std::swap(deps_, other.deps_);

	return *this;
}

service_base::stored_service service_base::find_service(const std::type_info& info) const
{
	for (const auto& service : this->deps_)
	{
		if (service->type( ) == info)
			return service;
	}

	return {};
}

void service_base::add_service_dependency(stored_service&& srv, const std::type_info& info)
{
	runtime_assert(find_service(info) == stored_service( ), "Service already stored!");
	this->deps_.push_back(std::move(srv));
}

service_state service_base::state() const
{
	return state_;
}

void service_base::reset()
{
	_Loading_access_assert(state_);
	deps_.clear( );
	state_ = service_state::unset;
}

service_base::load_result service_base::load(executor& ex) noexcept
{
	const auto set_error = [&]
	{
		state_ = service_state::error;
		return false;
	};

	switch (state_)
	{
		case service_state::unset:
		{
			const auto thread_id = std::this_thread::get_id( );
			const auto mtx       = co_await lock_.scoped_lock_async( );

			if (thread_id != std::this_thread::get_id( ))
			{
				co_return state_ == service_state::loaded;
			}

			co_await ex.schedule( );

			state_ = service_state::waiting;
			for (const auto& d : deps_)
			{
				if (!co_await d->load(ex))
				{
					runtime_assert("Unable to load other services!");
					co_return set_error( );
				}
			}
			state_ = service_state::loading;
			if (!co_await this->load_impl( ))
			{
				runtime_assert("Unable to load service!");
				co_return set_error( );
			}
			state_ = service_state::loaded;
			co_return true;
		}
		case service_state::waiting:
		case service_state::loading:
		{
			const auto mtx = co_await lock_.scoped_lock_async( );
			co_return state_ == service_state::loaded;
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

std::span<const service_base::stored_service> service_base::services() const
{
	return this->deps_;
}
