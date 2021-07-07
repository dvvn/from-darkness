#include "timer.h"

using namespace cheat::utl;

chrono::steady_clock::time_point timer::Now_( )
{
	return chrono::steady_clock::now( );
}

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
	start__.emplace(Now_( ));
	end__.reset( );
}

void timer::set_end( )
{
	BOOST_ASSERT_MSG(started(), "Timer not started");
	end__.emplace(Now_( ));
}

auto timer::elapsed( ) const
{
	BOOST_ASSERT_MSG(updated(), "Timer not updated");
	return *end__ - *start__;
}
