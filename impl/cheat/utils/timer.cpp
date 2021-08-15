#include "timer.h"

namespace cheat::utl
{
	timer::timer(bool start)
	{
		if (start)
			this->set_start( );
	}

	bool timer::started( ) const
	{
		return start__.has_value( );
	}

	bool timer::updated( ) const
	{
		return end__.has_value( );
	}

	void timer::set_start( )
	{
		start__.emplace(clock_type::now( ));
		end__.reset( );
	}

	void timer::set_end( )
	{
		runtime_assert(started(), "Timer not started");
		end__.emplace(clock_type::now( ));
	}

	timer::time_point::duration timer::elapsed( ) const
	{
		runtime_assert(updated(), "Timer not updated");
		return *end__ - *start__;
	}
}
