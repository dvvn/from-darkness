module;

#include "cheat/service/basic_includes.h"
#include "storage_includes.h"

module cheat.netvars;
import :storage;
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
import :data_dump;
#endif
import :data_fill;
import :data_handmade;

import cheat.csgo.modules;
import cheat.csgo.interfaces.C_BaseEntity;
import cheat.csgo.interfaces.EngineClient;
import cheat.csgo.interfaces.BaseClient;

using namespace cheat;
using namespace netvars_impl;
using namespace csgo;

struct netvars::impl
{
	netvars_storage storage;
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
	lazy::files_storage lazy;
#endif
};

netvars::netvars( ) = default;
netvars::~netvars( ) = default;

void netvars::construct( ) noexcept
{
	impl_ = std::make_unique<impl>( );
	this->deps( ).add<console>( );
}

bool netvars::load( ) noexcept
{
	auto& storage = impl_->storage;

	iterate_client_class(storage, IBaseClientDLL::get( ).GetAllClasses( ));

	const auto baseent = csgo_modules::client->find_vtable<C_BaseEntity>( );
	iterate_datamap(storage, baseent->GetDataDescMap( ));
	iterate_datamap(storage, baseent->GetPredictionDescMap( ));
	store_handmade_netvars(storage);

#ifdef CHEAT_NETVARS_RESOLVE_TYPE
	auto* const cons = this->deps( ).try_get<console>( );
	const auto log_result = log_netvars(cons, IVEngineClient::get( ).GetProductVersionString( ), storage);
	generate_classes(cons, log_result, storage, impl_->lazy);
#endif
	return true;
}

int netvars::at(const std::string_view & table, const std::string_view & item) const
{
	const auto& storage = impl_->storage;

	const auto target_class = storage.find(table);
	runtime_assert(target_class != storage.end( ));
	const auto netvar_info = target_class->find(item);
	runtime_assert(netvar_info != target_class->end( ));

	using namespace std::string_view_literals;
	return netvar_info->find("offset"sv)->get<int>( );
}

