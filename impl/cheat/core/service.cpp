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
	return value__ == loaded;
}

bool service_state::disabled( ) const
{
	switch (value__)
	{
		case loading:
		case loaded:
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
	state__ = static_cast<service_state>(other.state__);
	other.state__ = service_state::moved;
}

void service_base::operator=(service_base&& other) noexcept
{
	this->Loading_access_assert( );
	other.Loading_access_assert( );
	// ReSharper disable once CppRedundantCastExpression
	state__ = static_cast<service_state>(other.state__);
	other.state__ = service_state::moved;
}

service_state service_base::state( ) const
{
	return state__;
}

void service_base::load( )
{
	try
	{
		// ReSharper disable once CppRedundantCastExpression
		if (static_cast<service_state>(state__) != service_state::unset)
		{
			runtime_assert("Service loaded before");
			return;
		}

		state__ = service_state::loading;
		const auto loaded = this->Do_load( );
		state__ = service_state::loaded;

		CHEAT_CONSOLE_LOG("Service {}: {}", loaded ? "loaded " : "skipped", this->name( ));
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
