#pragma once

#include "detail.h"
#include "type_resolve.h"

#include "cheat/csgo/ClientClass.hpp"

namespace cheat::detail::netvars
{
	bool add_netvar_to_storage(netvars_storage& storage, const std::string_view& name, int offset, string_or_view_holder&& type);
	std::pair<netvars_storage::iterator, bool> add_child_class_to_storage(netvars_storage& storage, const std::string_view& name);
	void store_recv_props(netvars_storage& root_tree, netvars_storage& tree, const csgo::RecvTable* recv_table, int offset);
	void iterate_client_class(netvars_storage& root_tree, csgo::ClientClass* root_class);
	void store_datamap_props(netvars_storage& tree, csgo::datamap_t* map);
	void iterate_datamap(netvars_storage& root_tree, csgo::datamap_t* root_map);
}

#if _DEBUG || CHEAT_NETVARS_UPDATING
#define CHEAT_NETVARS_RESOLVE_TYPE
#endif
