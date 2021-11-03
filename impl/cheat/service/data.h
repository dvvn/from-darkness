#pragma once

#include "stored.h"
#include <vector>

namespace cheat
{
	struct basic_service_data : std::vector<stored_service<basic_service>>
	{
	};
}
