#include "netvars.h"
#include "detail.h"
#include "data_dumper.h"
#include "data_filler.h"
#include "data_handmade.h"

#include "cheat/core/console.h"
#include "cheat/core/csgo_interfaces.h"
#include "cheat/core/services_loader.h"
#include "cheat/core/csgo_modules.h"

#include "cheat/csgo/IBaseClientDll.hpp"
#include "cheat/csgo/entity/C_BaseEntity.h"

using namespace cheat;
using namespace csgo;

netvars_impl::netvars_impl( )
{
	data_ = std::make_unique<data_type>( );
	this->add_dependency(csgo_interfaces::get( ));
}

netvars_impl::~netvars_impl( ) = default;

basic_service::load_result netvars_impl::load_impl( ) noexcept
{
	detail::netvars::netvars_storage storage;

	iterate_client_class(storage, csgo_interfaces::get( )->client->GetAllClasses( ));

	const auto baseent = csgo_modules::client.find_vtable<C_BaseEntity>( );

	iterate_datamap(storage, baseent->GetDataDescMap( ));
	iterate_datamap(storage, baseent->GetPredictionDescMap( ));
	store_handmade_netvars(storage);

#ifdef CHEAT_NETVARS_RESOLVE_TYPE
	const auto info = log_netvars(storage);
	generate_classes(info, storage, data_->lazy);
#endif

	std::swap(data_->storage, storage);

	CHEAT_SERVICE_LOADED
}

int netvars_impl::at(const std::string_view& table, const std::string_view& item) const
{
	const auto& storage = data_->storage;

	const auto& target_class = storage.find(table);
	runtime_assert(target_class != storage.end());
	const auto& netvar_info = target_class->find(item);
	runtime_assert(netvar_info != target_class->end());

	using namespace std::string_view_literals;
	const auto basic_offset = netvar_info->find("offset"sv)->get<int>( );

	return basic_offset;
}

CHEAT_SERVICE_REGISTER(netvars);
