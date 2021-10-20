#pragma once
#include "cheat/core/service.h"

namespace cheat::features
{
	template <typename T>
	struct service_feature : service<T>
	{
		std::string_view name( ) const final { return nstd::type_name<T, "cheat::features">; }
		std::string_view debug_type( ) const final { return "Feature"; }
	};
}
