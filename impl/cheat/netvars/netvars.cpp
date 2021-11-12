#include "netvars.h"
#include "data_dumper.h"
#include "data_filler.h"
#include "data_handmade.h"
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
#include "lazy.h"
#include "storage.h"
#else
#include "storage_ndebug.h"
#endif
#include "storage_iter.h"

#include "cheat/core/console.h"
#include "cheat/core/csgo_interfaces.h"
#include "cheat/core/services_loader.h"
#include "cheat/core/csgo_modules.h"

#include "cheat/csgo/IBaseClientDll.hpp"
#include "cheat/csgo/entity/C_BaseEntity.h"

#include <cppcoro/task.hpp>

using namespace cheat;
using namespace detail::netvars;
using namespace csgo;

struct netvars_impl::data_type
{
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
	lazy_files_storage lazy;
#endif
	netvars_root_storage storage;
};

netvars_impl::netvars_impl( )
{
	data_ = std::make_unique<data_type>( );
	this->add_dependency(csgo_interfaces::get( ));
}

netvars_impl::~netvars_impl( ) = default;

auto netvars_impl::load_impl( ) noexcept -> load_result
{
	netvars_root_storage storage;

	iterate_client_class(storage, csgo_interfaces::get( )->client->GetAllClasses( ));

	const auto baseent = csgo_modules::client->find_vtable<C_BaseEntity>( );

	iterate_datamap(storage, baseent->GetDataDescMap( ));
	iterate_datamap(storage, baseent->GetPredictionDescMap( ));
	store_handmade_netvars(storage);

#ifdef CHEAT_NETVARS_RESOLVE_TYPE
	const auto info = log_netvars(storage);
	lazy_files_storage lazy;
	generate_classes(info, storage, lazy);
	std::swap(data_->lazy, lazy);
#endif
	std::swap(data_->storage, storage);

	CHEAT_SERVICE_LOADED
}

int netvars_impl::at(const std::string_view& table, const std::string_view& item) const
{
	const auto& storage = data_->storage;

	const auto target_class = storage.find(table NSTD_UTG);
	runtime_assert(target_class != storage.end( ));
	const auto netvar_info = unwrap_iter(target_class)->find(item NSTD_UTG);
	runtime_assert(netvar_info != unwrap_iter(target_class)->end( ));

#ifdef CHEAT_NETVARS_RESOLVE_TYPE
	using namespace std::string_view_literals;
	return netvar_info->find("offset"sv)->get<int>( );
#else
	return *unwrap_iter(netvar_info);
#endif
}

CHEAT_SERVICE_REGISTER(netvars);
