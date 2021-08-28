#include "service.h"

#include "console.h"

using namespace cheat;
using namespace detail;

template <typename T>
static void _Loading_access_assert([[maybe_unused]] T&& state)
{
	runtime_assert(state != service_state::loading, "Unable to modify service while loading!");
}

service_base::~service_base( )
{
	_Loading_access_assert(state_);
}

service_base::service_base(service_base&& other) noexcept
{
	*this = std::move(other);
}

void service_base::operator=(service_base&& other) noexcept
{
	_Loading_access_assert(this->state_);
	_Loading_access_assert(other.state_);

	state_       = std::move(other.state_);
	other.state_ = service_state::moved;
	std::swap(this->services_, other.services_);
}

service_state service_base::state( ) const
{
	return state_;
}

void service_base::reset( )
{
	_Loading_access_assert(this->state_);
	this->services_.clear( );
	this->state_ = service_state::moved;
}

template <typename T, typename T1>
static void _Service_msg([[maybe_unused]] const T* owner, [[maybe_unused]] T1&& msg)
{
	CHEAT_CONSOLE_LOG("{} service: {}", owner->name( ), msg);
}

bool service_base::validate_init_state( ) const
{
	if (state_ == init_state_)
		return true;

	runtime_assert("Unable to validate state!");
	return false;
}

service_base::load_result service_base::load(executor& ex)
{
	if (!validate_init_state( ))
		co_return state_;

	state_ = service_state::waiting;
	if (!co_await this->wait_for_others(ex))
	{
		_Service_msg(this, "Error while child waiting");
		co_return state_ = service_state::error;
	}
	co_await ex.schedule( );
	state_ = service_state::loading;
	switch ((co_await this->load_impl( )).value( ))
	{
		case service_state::loaded:
		{
			this->after_load( );
			co_return state_ = service_state::loaded;
		}
		case service_state::skipped:
		{
			this->after_skip( );
			co_return state_ = service_state::skipped;
		}
		default:
		{
			this->after_error( );
			co_return state_ = service_state::error;
		}
	}
}

service_base::child_wait_result service_base::wait_for_others(executor& ex)
{
	for (auto itr = services_.begin( ); itr != services_.end( );)
	{
		const auto se = *itr;
		switch (se->state( ).value( ))
		{
			case service_state::unset:
				co_await ex.schedule( );
				co_await se->load(ex);
				continue;
			case service_state::loaded:
			case service_state::skipped:
				break;
			case service_state::moved:
			case service_state::error:
				co_return false;
			case service_state::waiting:
			case service_state::loading:
				runtime_assert("Service still loading!");
				break;
			default:
				runtime_assert("Update function!");
				break;
		}

		++itr;
	}

	co_return true;
}

void service_base::after_load( )
{
	_Service_msg(this, "loaded");
}

void service_base::after_skip( )
{
	_Service_msg(this, "skipped");
}

void service_base::after_error( )
{
	_Service_msg(this, "error while loading");
}

void service_base::set_state(service_state&& state)
{
	_Loading_access_assert(state_);
	state_ = std::move(state);
}

std::span<const service_base::shared_service> service_base::services( ) const
{
	return services_;
}

service_base::load_result service_hook_helper::load_impl( )
{
	co_return this->hook( ) && this->enable( ) ? service_state::loaded : service_state::error;
}

service_base::load_result service_always_skipped::load([[maybe_unused]] executor& ex)
{
	(void)this->validate_init_state( );
	auto state = service_state::skipped;
	this->set_state(state);
	this->after_skip( );
	co_return state;
}
