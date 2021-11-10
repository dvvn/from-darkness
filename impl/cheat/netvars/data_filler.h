#pragma once
#include <string_view>

namespace cheat::csgo
{
	class RecvTable;
	struct datamap_t;
	class ClientClass;
}

namespace cheat::detail::netvars
{
	struct string_or_view_holder;
	struct netvars_root_storage_iter;
	struct netvars_storage;
	struct netvars_root_storage;

	bool add_netvar_to_storage(netvars_storage& storage, const std::string_view& name, int offset, string_or_view_holder&& type);
	std::pair<netvars_root_storage_iter, bool> add_child_class_to_storage(netvars_root_storage& storage, const std::string_view& name);
	void store_recv_props(netvars_root_storage& root_tree, netvars_storage& tree, const csgo::RecvTable* recv_table, int offset);
	void iterate_client_class(netvars_root_storage& root_tree, csgo::ClientClass* root_class);
	void store_datamap_props(netvars_storage& tree, csgo::datamap_t* map);
	void iterate_datamap(netvars_root_storage& root_tree, csgo::datamap_t* root_map);
}

#if  defined(_DEBUG) || defined(CHEAT_NETVARS_UPDATING)
#define CHEAT_NETVARS_RESOLVE_TYPE
#endif
