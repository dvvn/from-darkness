#include "service.h"

#include "console.h"

using namespace cheat;
using namespace detail;
using namespace utl;

[[maybe_unused]]
static std::future<void> _Get_ready_task( )
{
	std::promise<void> pr;
	pr.set_value( );
	return pr.get_future( );
}

template <std::derived_from<std::exception> Ex>
[[maybe_unused]]
static std::future<void> _Get_error_task(Ex&& ex)
{
	std::promise<void> pr;
	pr.set_exception(move(ex));
	return pr.get_future( );
}

bool service_state::operator!( ) const
{
	return value_ == unset;
}

bool service_state::done( ) const
{
	switch (value_)
	{
		case loaded:
		case skipped:
#ifdef BOOST_THREAD_PROVIDES_INTERRUPTIONS
		case stopped:
#endif
			return true;
		default:
			return false;
	}
}

bool service_state::disabled( ) const
{
	switch (value_)
	{
		case loading:
		case loaded:
		case skipped:
			return false;
		default:
			return true;
	}
}

template <typename T>
FORCEINLINE static void _Loading_access_assert(T&& state)
{
	runtime_assert(static_cast<service_state>(state) != service_state::loading, "Unable to modify service while loading!");
}

service_base::~service_base( )
{
	_Loading_access_assert(state_);
}

service_base::service_base(service_base&& other) noexcept
{
	_Loading_access_assert(other.state_);
	// ReSharper disable once CppRedundantCastExpression
	state_       = static_cast<service_state>(other.state_);
	other.state_ = service_state::moved;
}

void service_base::operator=(service_base&& other) noexcept
{
	_Loading_access_assert(this->state_);
	_Loading_access_assert(other.state_);

	// ReSharper disable once CppRedundantCastExpression
	state_       = static_cast<service_state>(other.state_);
	other.state_ = service_state::moved;
}

service_state service_base::state( ) const
{
	return state_;
}

template <typename T>
FORCEINLINE static void _Service_loaded_msg([[maybe_unused]] const T* owner, [[maybe_unused]] bool loaded)
{
	CHEAT_CONSOLE_LOG("Service {}: {}", loaded ? "loaded" : "skipped", owner->name( ));
}

void service_base::load( )
{
	try
	{
		if (static_cast<service_state>(state_) != service_state::unset)
		{
			runtime_assert("Service loaded before");
			return;
		}

		state_            = service_state::loading;
		const auto loaded = this->load_impl( );
		state_            = service_state::loaded;

		_Service_loaded_msg(this, loaded);
		this->after_load( );
	}
#ifdef BOOST_THREAD_PROVIDES_INTERRUPTIONS
	catch (const thread_interrupted&ex)
	{
		state_ = service_state::stopped;
		CHEAT_CONSOLE_LOG("Service {} forced to be stoped {}. {}", ex.what( ));
		this->after_stop(ex);
	}
#endif
	catch (const std::exception& ex)
	{
		state_ = service_state::error;
		CHEAT_CONSOLE_LOG("Unable to load service {}. {}", this->name( ), ex.what( ));
		this->after_error(ex);
	}
}

bool service_hook_helper::load_impl( )
{
	this->hook( );
	this->enable( );

	return true;
}

service_always_skipped::service_always_skipped( )
{
	state_ = service_state::skipped;
}

void service_always_skipped::load( )
{
	if (static_cast<service_state>(state_) != service_state::skipped)
	{
		runtime_assert("Service must be skipped, but state is changed");
		return;
	}

	_Service_loaded_msg(this, false);
}
