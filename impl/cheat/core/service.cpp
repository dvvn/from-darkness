#include "service.h"

#include "console.h"

using namespace cheat;
using namespace detail;
using namespace utl;

[[maybe_unused]] static std::future<void> _Get_ready_task( )
{
	std::promise<void> pr;
	pr.set_value( );
	return pr.get_future( );
}

template <std::derived_from<std::exception> Ex>
[[maybe_unused]] static std::future<void> _Get_error_task(Ex&& ex)
{
	std::promise<void> pr;
	pr.set_exception(move(ex));
	return pr.get_future( );
}

bool service_state::operator!( ) const
{
	return value__ == unset;
}

bool service_state::done( ) const
{
	switch (value__)
	{
		case loaded:
		case skipped:
			return true;
		default:
			return false;
	}
}

bool service_state::disabled( ) const
{
	switch (value__)
	{
		case loading:
		case loaded:
		case skipped:
			return false;
		default:
			return true;
	}
}

void service_base::Loading_access_assert( ) const
{
	runtime_assert(static_cast<service_state>(state__) != service_state::loading, "Unable to modify service while loading!");
	(void)this;
}

service_base::~service_base( )
{
	Loading_access_assert( );
}

service_base::service_base(service_base&& other) noexcept
{
	other.Loading_access_assert( );
	// ReSharper disable once CppRedundantCastExpression
	state__       = static_cast<service_state>(other.state__);
	other.state__ = service_state::moved;
}

void service_base::operator=(service_base&& other) noexcept
{
	this->Loading_access_assert( );
	other.Loading_access_assert( );
	// ReSharper disable once CppRedundantCastExpression
	state__       = static_cast<service_state>(other.state__);
	other.state__ = service_state::moved;
}

service_state service_base::state( ) const
{
	return state__;
}

template <typename T>
FORCEINLINE static void _Service_loaded_msg([[maybe_unused]] const T* owner, [[maybe_unused]] bool loaded)
{
	CHEAT_CONSOLE_LOG("Service {}: {}", loaded ? "loaded " : "skipped", owner->name( ));
}

void service_base::load( )
{
	try
	{
		if (static_cast<service_state>(state__) != service_state::unset)
		{
			runtime_assert("Service loaded before");
			return;
		}

		state__           = service_state::loading;
		const auto loaded = this->Do_load( );
		state__           = service_state::loaded;

		_Service_loaded_msg(this, loaded);
		this->On_load( );
	}
#if defined BOOST_THREAD_PROVIDES_INTERRUPTIONS
	catch (const thread_interrupted&)
	{
		state__ = service_state::stopped;
		this->On_stop( );
	}
#endif
	catch ([[maybe_unused]] const std::exception& ex)
	{
		state__ = service_state::error;
		CHEAT_CONSOLE_LOG("Unable to load service {}. {}", this->name( ), ex.what( ));
		this->On_error( );
	}
}

service_skipped_always::service_skipped_always( )
{
	state__ = service_state::skipped;
}

void service_skipped_always::load( )
{
	if (static_cast<service_state>(state__) != service_state::skipped)
	{
		runtime_assert("Service must be skipped, but state is different");
		return;
	}

	_Service_loaded_msg(this, false);
}
