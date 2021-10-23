#pragma once
#include "detail.h"

namespace cheat::detail::netvars
{
	enum class log_info :uint8_t
	{
		unset
	  , skipped
	  , created
	  , updated
	};

	struct include_name : std::string
	{
		include_name( ) = default;

		template <typename T>
		include_name(T&& name, bool global)
			: std::string(std::forward<T>(name)), global(global)
		{
		}

		bool global;
	};

	log_info log_netvars(const netvars_storage& netvars_data);
	void generate_classes(log_info info, netvars_storage& netvars_data, lazy_files_storage& lazy_storage);
}
