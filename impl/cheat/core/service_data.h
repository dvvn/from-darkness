#pragma once

#include "stored_service.h"
#include <vector>

namespace cheat
{
	struct basic_service_data : std::vector<stored_service<>>
	{
	};
}
