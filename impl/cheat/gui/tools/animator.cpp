#include "animator.h"

#include "nstd/runtime assert.h"

#include <imgui.h>

using namespace cheat;
using namespace gui::tools;

int8_t animator::dir( ) const
{
	return dir_;
}

bool animator::updating( ) const
{
	const auto val = value_.current;
	return val > value_.min && val < value_.max;
}

void animator::set(int8_t direction)
{
	runtime_assert(direction == 1 || direction == -1);
	if (!updating( ))
	{
		time_          = time_max_;
		value_.current = get_limit(-direction);
	}
	dir_ = direction;
}

static animator::float_type _Get_percentage_diff(animator::float_type a, animator::float_type b)
{
	return ((b - a) * static_cast<animator::float_type>(100.0)) / a;
}

static animator::float_type _Get_precent_from_num(animator::float_type percent, animator::float_type num)
{
	return num * percent / static_cast<animator::float_type>(100.0);
}

bool animator::update( )
{
	if (time_ > 0)
	{
		const auto delta = ImGui::GetIO( ).DeltaTime;

		time_ -= delta;
		if (time_ > static_cast<float_type>(0))
		{
			const auto diff = _Get_percentage_diff(time_, time_max_);
			const auto val  = _Get_precent_from_num(diff, value_.max - value_.min) + value_.min;
			value_.current  = dir_ == 1 ? val : static_cast<float_type>(1.0) - val;
			if (updating( ))
				return true;
		}

		finish( );
	}
	return false;
}

void animator::finish( )
{
	time_          = 0;
	value_.current = get_limit(dir_);
}

void animator::restart( )
{
	//finish(  );
	set(dir_);
}

bool animator::done( ) const
{
	return value_.current == get_limit(dir_);
}

bool animator::done(int8_t direction) const
{
	bool ret;

	if (dir_ == direction)
	{
		ret = done( );
		//-
	}
	else
	{
		auto& dir = const_cast<int8_t&>(dir_);

		dir = direction;
		ret = done( );
		dir = -direction;
	}
	return ret;
}

animator::float_type animator::value( ) const
{
	return value_.current;
}

animator::animator(float_type value_min, float_type value_max, float_type time_max)
{
	runtime_assert(value_max <= 1);

	set_min_max(value_min, value_max);
	set_time(time_max);
}

void animator::set_min(float_type val)
{
	runtime_assert(val < value_.max);
	runtime_assert(val >= 0);
	value_.min = val;
}

void animator::set_max(float_type val)
{
	runtime_assert(val <= 1);
	runtime_assert(val > value_.min);
	value_.max = val;
}

void animator::set_min_max(float_type min, float_type max)
{
	runtime_assert(min < max);
	runtime_assert(min >= 0);
	runtime_assert(max <= 1);
	value_.min = min;
	value_.max = max;
}

void animator::set_time(float_type val)
{
	runtime_assert(val > 0);
	time_max_ = val;
}

animator::float_type animator::min( ) const
{
	return value_.min;
}

animator::float_type animator::max( ) const
{
	return value_.max;
}

animator::float_type animator::time( ) const
{
	return time_max_;
}

//auto animator::setup_limits(float_type value_min, float_type value_max, float_type time_max) -> void
//{
//	runtime_assert(value_min < value_max);
//	runtime_assert(value_min >= 0);
//	runtime_assert(value_max <= 1);
//	runtime_assert(time_max > 0);
//	value_.min = value_min;
//	value_.max = value_max;
//	time_max_ = time_max;
//}

animator::float_type animator::get_limit(int8_t dir) const
{
	return /*dir_*/dir == 1 ? value_.max : value_.min;
}
