module;

#include "storage_includes.h"
#include "cheat/console/includes.h"

export module cheat.netvars:data_dump;
export import :storage;
export import :lazy;
export import cheat.console;

export namespace cheat::netvars_impl
{
	bool log_netvars(console*logger,const netvars_storage& root_netvars_data);
	void generate_classes(console*logger,bool recreate, netvars_storage& root_netvars_data, lazy::files_storage& lazy_storage);
}	