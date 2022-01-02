module;

#include "cheat/service/includes.h"
#include "storage_includes.h"

module cheat.netvars;
import :data_fill;
import :data_handmade;
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
import :data_dump;
#endif
import cheat.csgo.interfaces;
import cheat.csgo.modules;
import cheat.csgo.structs.BaseClient;

using namespace cheat;
using namespace netvars_impl;
using namespace csgo;

netvars::netvars( ) = default;
netvars::~netvars( ) = default;

bool netvars::load_impl( ) noexcept
{
	iterate_client_class(storage_, csgo_interfaces::get( )->client->GetAllClasses( ));

	/*const auto baseent = csgo_modules::client->find_vtable<C_BaseEntity>( );
	iterate_datamap(storage, baseent->GetDataDescMap( ));
	iterate_datamap(storage, baseent->GetPredictionDescMap( ));*/
	store_handmade_netvars(storage_);

#ifdef CHEAT_NETVARS_RESOLVE_TYPE
	const auto info = log_netvars(storage_);
	generate_classes(info, storage_, lazy_);
#endif
	return true;
}

int netvars::at(const std::string_view & table, const std::string_view & item) const
{
	const auto target_class = storage_.find(table);
	runtime_assert(target_class != storage_.end( ));
	const auto netvar_info = target_class->find(item);
	runtime_assert(netvar_info != target_class->end( ));

	using namespace std::string_view_literals;
	return netvar_info->find("offset"sv)->get<int>( );
}

//CHEAT_SERVICE_REGISTER(netvars);
