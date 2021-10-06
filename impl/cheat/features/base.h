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

#define CHEAT_FEATURE_INIT_1 CHEAT_SERVICE_LOADED
#define CHEAT_FEATURE_INIT_0 CHEAT_SERVICE_SKIPPED

#define CHEAT_FEATURE_INIT(FT) _CONCAT(CHEAT_FEATURE_INIT_,FT)
#define CHEAT_FEATURE_CALL_BLOCKER\
	runtime_assert("Disable but called");\
	__pragma(message(__FUNCTION__": disabled"))\
	(void)this;