#include "service.h"

#include "console.h"

using namespace cheat;
using namespace detail;
using namespace utl;
using namespace future_state;

[[maybe_unused]] static future<void> _Get_ready_task( )
{
	promise<void> pr;
	pr.set_value( );
	return pr.get_future( );
}

template <std::derived_from<std::exception> Ex>
[[maybe_unused]] static future<void> _Get_error_task(Ex&& ex)
{
	promise<void> pr;
	pr.set_exception(move(ex));
	return pr.get_future( );
}

bool service_state2::operator!( ) const
{
	return value__ == unset;
}

bool service_state2::done( ) const
{
	return value__ == loaded;
}

bool service_state2::disabled( ) const
{
	switch (value__)
	{
		case unset:
		case stopped:
		case error:
			return true;
		default:
			return false;
	}
}

void service_base2::Loading_access_assert( ) const
{
	BOOST_ASSERT_MSG(state__ != service_state2::loading, "Unable to modify service while loading!");
	(void)this;
}

service_base2::~service_base2( )
{
	Loading_access_assert( );
}

service_base2::service_base2(service_base2&& other) noexcept
{
	other.Loading_access_assert( );
	state__ = other.state__;
}

void service_base2::operator=(service_base2&& other) noexcept
{
	this->Loading_access_assert( );
	other.Loading_access_assert( );
	state__ = other.state__;
}

service_state2 service_base2::state( ) const
{
	return state__;
}

void service_base2::load( )
{
	try
	{
		if (state__ != service_state2::unset)
		{
			BOOST_ASSERT("Service loaded before");
			return;
		}

		state__ = service_state2::loading;
		const auto loaded = this->Do_load( );
		state__ = service_state2::loaded;

#ifdef CHEAT_HAVE_CONSOLE
		_Log_to_console(format("Service {}: {}", loaded ? "loaded " : "skipped", this->name( )));
#endif
		this->On_load( );
	}
#if defined BOOST_THREAD_PROVIDES_INTERRUPTIONS
	catch (const thread_interrupted&)
	{
		state__ = service_state2::stopped;
		this->On_stop( );
	}
#endif
	catch ([[maybe_unused]] const std::exception& ex)
	{
		state__ = service_state2::error;
#ifdef CHEAT_HAVE_CONSOLE
		_Log_to_console(format("Unable to load service {}. {}", this->name( ), ex.what( )));
#endif
		this->On_error( );
	}
}
