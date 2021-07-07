#include "animator.h"

using namespace cheat;
using namespace gui;
using namespace hooks;
using namespace utl;

auto animator::dir( ) const -> int
{
	return dir__;
}

auto animator::updating( ) const -> bool
{
	const auto val = value__.current;
	return val > value__.min && val < value__.max;
}

auto animator::set(int direction) -> void
{
	BOOST_ASSERT(direction == 1 || direction == -1);
	if (!updating( )) 
	{
		time__ = time_max__;
		value__.current = Limit_(-direction);
	}
	dir__ = direction;
}

auto animator::update( ) -> bool
{
	if (time__ > 0)
	{
		const auto& delta = ImGui::GetIO( ).DeltaTime;
		const auto  step = (value__.max - value__.min) * delta / time_max__;
		time__ -= delta;
		value__.current += step * dir__;
		if (time__ > 0 && updating( ))
			return true;
		finish( );
	}
	return false;
}

auto animator::finish( ) -> void
{
	time__ = 0;
	value__.current = Limit_(dir__);
}

auto animator::done( ) const -> bool
{
	return value__.current == Limit_(dir__);
}

auto animator::done(int direction) const -> bool
{
	if (dir__ == direction)
		return done( );
	auto& dir = const_cast<int&>(dir__);
	dir = direction;
	const auto ret = done( );
	dir = -direction;
	return ret;
}

auto animator::value( ) const -> float
{
	return value__.current;
}

animator::animator(float value_min, float value_max, float time_max)
{
	BOOST_ASSERT(value_min < value_max);
	BOOST_ASSERT(value_min >= 0);
	BOOST_ASSERT(value_max <= 1);
	BOOST_ASSERT(time_max > 0);
	value__.min = value_min;
	value__.max = value_max;
	time_max__ = time_max;
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

auto animator::Limit_(float dir) const -> float
{
	return (/*dir__*/dir == 1 ? value__.max : value__.min);
}
