#pragma once
#include "detail.h"

namespace cheat::detail
{
	enum class dump_info :uint8_t
	{
		unset
	  , skipped
	  , created
	  , updated
	};

	struct include_name : std::string
	{
		template <typename T>
		include_name(T&& name, bool global)
			: std::string(std::forward<T>(name)), global(global)
		{
		}

		bool global;
	};

	dump_info _Dump_netvars(const netvars_storage& netvars_data);
	void _Generate_classes(dump_info info, netvars_storage& netvars_data, lazy_files_storage& lazy_storage);
}
