//#include "netvars.h"
//#include "data_filler.h"
//#include "data_handmade.h"
//#ifdef CHEAT_NETVARS_RESOLVE_TYPE
//#include "data_dumper.h"
//#include "lazy.h"
//#include "storage.h"
//#else
//#include "storage_ndebug.h"
//#endif
//#include "storage_iter.h"
//
//#include "cheat/core/console.h"
//#include "cheat/core/csgo_interfaces.h"
//#include "cheat/core/services_loader.h"
//#include "cheat/core/csgo_modules.h"
//
//#include "cheat/csgo/IBaseClientDll.hpp"
//#include "cheat/csgo/entity/C_BaseEntity.h"
//
//#include <cppcoro/task.hpp>

module;

#include "cheat/service/includes.h"
#include "storage_includes.h"

module cheat.netvars;
import :data_fill;
import :data_dump;
import cheat.csgo.interfaces;
import cheat.csgo.modules;
import cheat.csgo.structs.BaseClient;

using namespace cheat;
using namespace netvars_impl;
using namespace csgo;

struct netvars::data_type
{
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
	lazy::files_storage lazy;
#endif
	netvars_root_storage storage;
};

netvars::netvars( )
{
	data_ = std::make_unique<data_type>( );
	this->add_dependency(csgo_interfaces::get( ));
}

netvars::~netvars( ) = default;

bool netvars::load_impl( ) noexcept 
{
#if 0//TEMPORARY
	netvars_root_storage storage;
	iterate_client_class(storage, csgo_interfaces::get( )->client->GetAllClasses( ));

	const auto baseent = csgo_modules::client->find_vtable<C_BaseEntity>( );
	iterate_datamap(storage, baseent->GetDataDescMap( ));
	iterate_datamap(storage, baseent->GetPredictionDescMap( ));
	store_handmade_netvars(storage);

#ifdef CHEAT_NETVARS_RESOLVE_TYPE
	const auto info = log_netvars(storage);
	lazy::files_storage lazy;
	generate_classes(info, storage, lazy);
	std::swap(data_->lazy, lazy);
#endif
	std::swap(data_->storage, storage);
#endif
	return true;
}

int netvars::at(const std::string_view& table, const std::string_view& item) const
{
	const auto& storage = data_->storage;

	const iterator_wrapper target_class = storage.find(table);
	runtime_assert(target_class != storage.end( ));
	const iterator_wrapper netvar_info = target_class->find(item);
	runtime_assert(netvar_info != target_class->end( ));

#ifdef CHEAT_NETVARS_RESOLVE_TYPE
	using namespace std::string_view_literals;
	return netvar_info->find("offset"sv)->get<int>( );
#else
	return *netvar_info;
#endif
}

//CHEAT_SERVICE_REGISTER(netvars);
