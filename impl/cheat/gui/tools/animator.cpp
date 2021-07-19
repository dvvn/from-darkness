#include "animator.h"

using namespace cheat;
using namespace gui::tools;
using namespace utl;

int animator::dir( ) const
{
	return dir__;
}

bool animator::updating( ) const
{
	const auto val = value__.current;
	return val > value__.min && val < value__.max;
}

void animator::set(int direction)
{
	BOOST_ASSERT(direction == 1 || direction == -1);
	if (!updating( ))
	{
		time__ = time_max__;
		value__.current = Limit_(-direction);
	}
	dir__ = direction;
}

bool animator::update( )
{
	if (time__ > 0)
	{
		const auto delta = ImGui::GetIO( ).DeltaTime;
		const auto step = (value__.max - value__.min) * delta / time_max__;
		time__ -= delta;
		value__.current += step * dir__;
		if (time__ > 0 && updating( ))
			return true;
		finish( );
	}
	return false;
}

void animator::finish( )
{
	time__ = 0;
	value__.current = Limit_(dir__);
}

bool animator::done( ) const
{
	return value__.current == Limit_(dir__);
}

bool animator::done(int direction) const
{
	if (dir__ == direction)
		return done( );
	auto& dir = const_cast<int&>(dir__);
	dir = direction;
	const auto ret = done( );
	dir = -direction;
	return ret;
}

float animator::value( ) const
{
	return value__.current;
}

animator::animator(float value_min, float value_max, float time_max)
{
	BOOST_ASSERT(value_max <= 1);

	set_min_max(value_min, value_max);
	set_time(time_max);
}

void animator::set_min(float val)
{
	BOOST_ASSERT(val < value__.max);
	BOOST_ASSERT(val >= 0);
	value__.min = val;
}

void animator::set_max(float val)
{
	BOOST_ASSERT(val <= 1);
	BOOST_ASSERT(val > value__.min);
	value__.max = val;
}

void animator::set_min_max(float min, float max)
{
	BOOST_ASSERT(min < max);
	BOOST_ASSERT(min >= 0);
	BOOST_ASSERT(max <= 1);
	value__.min = min;
	value__.max = max;
}

void animator::set_time(float val)
{
	BOOST_ASSERT(val>0);
	time_max__ = val;
}

float animator::min( ) const
{
	return value__.min;
}

float animator::max( ) const
{
	return value__.max;
}

float animator::time( ) const
{
	return time_max__;
}

//auto animator::setup_limits(float value_min, float value_max, float time_max) -> void
//{
//	BOOST_ASSERT(value_min < value_max);
//	BOOST_ASSERT(value_min >= 0);
//	BOOST_ASSERT(value_max <= 1);
//	BOOST_ASSERT(time_max > 0);
//	value__.min = value_min;
//	value__.max = value_max;
//	time_max__ = time_max;
//}

float animator::Limit_(float dir) const
{
	return (/*dir__*/dir == 1 ? value__.max : value__.min);
}
