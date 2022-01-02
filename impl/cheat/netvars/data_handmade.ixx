module;

#include "storage_includes.h"

export module cheat.netvars:data_handmade;
export import :storage;

export namespace cheat::netvars_impl
{
	void store_handmade_netvars(netvars_storage& root_tree);
}
