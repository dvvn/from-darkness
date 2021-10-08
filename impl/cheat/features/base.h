#pragma once
#include "cheat/core/service.h"

namespace cheat::features
{
	template <typename T>
	struct service_feature : service<T>
	{
		std::string_view name() const final { return nstd::type_name<T, "cheat::features">; }
		service_base::debug_info_t debug_info() const final { return {"Feature", "initialized", "disabled", "NOT initialized"}; }
	};
}

