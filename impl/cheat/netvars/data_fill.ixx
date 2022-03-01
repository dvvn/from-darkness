module;

#include "storage_includes.h"

export module cheat.netvars.data_fill;
export import cheat.netvars.storage;
export import cheat.netvars.type_resolve;
export import cheat.csgo.structs.ClientClass;

export namespace cheat::netvars
{
	bool add_netvar_to_storage(storage& storage, const std::string_view name, int offset, string_or_view_holder&& type);
	//std::pair<iterator_wrapper<X::iterator>,bool>
	using add_child_t = std::pair<storage::iterator,bool>;
	add_child_t add_child_class_to_storage(storage& root_storage, const std::string_view name);
	void store_recv_props(storage& root_tree, storage& tree, const csgo::RecvTable* recv_table, int offset);
	void iterate_client_class(storage& root_tree, csgo::ClientClass* root_class);
	void store_datamap_props(storage& tree, csgo::datamap_t* map);
	void iterate_datamap(storage& root_tree, csgo::datamap_t* root_map);
}