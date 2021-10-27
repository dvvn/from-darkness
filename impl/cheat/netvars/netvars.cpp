#include "netvars.h"
#include "detail.h"
#ifndef CHEAT_NETVARS_DUMPER_DISABLED
#include "data_dumper.h"
#include "data_filler.h"
#include "data_handmade.h"
#endif

#include "cheat/core/console.h"
#include "cheat/core/csgo_interfaces.h"
#include "cheat/core/services_loader.h"
#ifndef CHEAT_NETVARS_DUMPER_DISABLED
#include "cheat/core/csgo_modules.h"
#include "cheat/csgo/IBaseClientDll.hpp"
#include "cheat/csgo/entity/C_BaseEntity.h"
#endif

using namespace cheat;
using namespace csgo;

netvars::netvars( )
{
	data_ = std::make_unique<data_type>( );
	this->wait_for_service<csgo_interfaces>( );
}

netvars::~netvars( ) = default;

service_impl::load_result netvars::load_impl( ) noexcept
{
#ifdef CHEAT_NETVARS_DUMPER_DISABLED
	CHEAT_SERVICE_SKIPPED
#else

	auto& data = data_->storage;

#ifdef _DEBUG
	data.clear( );
#endif

	const auto interfaces = csgo_interfaces::get_ptr( );

	iterate_client_class(data, interfaces->client->GetAllClasses( ));

	const auto baseent = csgo_modules::client.find_vtable<C_BaseEntity>( );

	iterate_datamap(data, baseent->GetDataDescMap( ));
	iterate_datamap(data, baseent->GetPredictionDescMap( ));
	store_handmade_netvars(data);

#if defined(CHEAT_NETVARS_RESOLVE_TYPE)
	const auto info = log_netvars(data_->storage);
	generate_classes(info, data_->storage, data_->lazy);
#endif

	CHEAT_SERVICE_LOADED
#endif
}

int netvars::at([[maybe_unused]] const std::string_view& table, [[maybe_unused]] const std::string_view& item) const
{
#ifdef CHEAT_NETVARS_DUMPER_DISABLED
	CHEAT_CALL_BLOCKER
	return 0;
#else
	const auto& storage = data_->storage;

	const auto& target_class = storage.find(table);
	runtime_assert(target_class!=storage.end());
	const auto& netvar_info = target_class->find(item);
	runtime_assert(netvar_info!=target_class->end());

	using namespace std::string_view_literals;
	const auto basic_offset = netvar_info->find("offset"sv)->get<int>( );

	return basic_offset;
#endif
}

CHEAT_REGISTER_SERVICE(netvars);
