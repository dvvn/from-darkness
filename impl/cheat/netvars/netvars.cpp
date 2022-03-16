module;

#include <nstd/runtime_assert.h>

#include "storage_includes.h"

module cheat.netvars;
import cheat.netvars.storage;
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
import cheat.netvars.data_dump;
#endif
import cheat.netvars.data_fill;
import cheat.netvars.data_handmade;
import cheat.console.object_message;
import cheat.csgo.modules;
import cheat.csgo.interfaces.C_BaseEntity;
import cheat.csgo.interfaces.EngineClient;
import cheat.csgo.interfaces.BaseClient;

using namespace cheat;
using namespace netvars;
using namespace csgo;

struct netvars_holder :console::object_message<netvars_holder>
{
	storage storage;
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
	lazy::files_storage lazy;
#endif

	netvars_holder( )
	{
		iterate_client_class(storage, IBaseClientDLL::get( ).GetAllClasses( ));

		const auto baseent = csgo_modules::client.find_vtable<C_BaseEntity>( );
		iterate_datamap(storage, baseent->GetDataDescMap( ));
		iterate_datamap(storage, baseent->GetPredictionDescMap( ));
		store_handmade_netvars(storage);

#ifdef CHEAT_NETVARS_RESOLVE_TYPE
		const auto log_result = log_netvars(IVEngineClient::get( ).GetProductVersionString( ), storage);
		generate_classes(log_result, storage, lazy);
#endif
	}
};

std::string_view console::object_message<netvars_holder>::_Name( ) const
{
	return "netvars";
}

int netvars::get_offset(const std::string_view table, const std::string_view item)
{
	const auto& storage = nstd::one_instance<netvars_holder>::get( ).storage;

	const auto target_class = storage.find(table);
	runtime_assert(target_class != storage.end( ));
	const auto netvar_info = target_class->find(item);
	runtime_assert(netvar_info != target_class->end( ));

	using namespace std::string_view_literals;
	return netvar_info->find("offset"sv)->get<int>( );
}

