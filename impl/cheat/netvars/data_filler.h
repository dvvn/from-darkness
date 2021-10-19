#pragma once

#include "detail.h"
#include "type_resolve.h"

#include "cheat/csgo/ClientClass.hpp"

namespace cheat::detail
{
	bool _Save_netvar(netvars_storage& storage, const std::string_view& name, int offset, netvar_type_holder&& type);
	std::pair<netvars_storage::iterator, bool> _Add_child_class(netvars_storage& storage, const std::string_view& name);
	void _Store_recv_props(netvars_storage& root_tree, netvars_storage& tree, const csgo::RecvTable* recv_table, int offset);
	void _Iterate_client_class(netvars_storage& root_tree, csgo::ClientClass* root_class);
	void _Store_datamap_props(netvars_storage& tree, csgo::datamap_t* map);
	void _Iterate_datamap(netvars_storage& root_tree, csgo::datamap_t* root_map);
}
