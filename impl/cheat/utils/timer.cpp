#include "timer.h"

using namespace cheat::utl;

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
	BOOST_ASSERT_MSG(started(), "Timer not started");
	end__.emplace(clock_type::now( ));
}

timer::time_point::duration timer::elapsed( ) const
{
	BOOST_ASSERT_MSG(updated(), "Timer not updated");
	return *end__ - *start__;
}
