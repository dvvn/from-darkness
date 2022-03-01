module;

#include "storage_includes.h"

export module cheat.netvars.data_handmade;
export import cheat.netvars.storage;

export namespace cheat::netvars
{
	void store_handmade_netvars(storage& root_tree);
}
