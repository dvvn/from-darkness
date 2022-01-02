module;

#include "storage_includes.h"

export module cheat.netvars:data_dump;
export import :storage;
export import :lazy;

export namespace cheat::netvars_impl
{
	enum class log_info :uint8_t
	{
		unset
		, skipped
		, created
		, updated
	};

	log_info log_netvars(const netvars_storage& root_netvars_data);
	void generate_classes(log_info info, netvars_storage& root_netvars_data, lazy::files_storage& lazy_storage);
}	