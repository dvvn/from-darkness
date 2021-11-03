#pragma once
#include <cstdint>

namespace cheat
{
	enum class service_state : uint8_t
	{
		unset = 0
	  , waiting
	  , loading
	  , loaded
	  , error
	};
}
