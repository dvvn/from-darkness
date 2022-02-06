module;

#include "cheat/service/basic_includes.h"
#include "storage_includes.h"

module cheat.netvars;
import :data_fill;
import :data_handmade;
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
import :data_dump;
#endif
import cheat.csgo.interfaces;
import cheat.csgo.modules;
import cheat.csgo.interfaces;

using namespace cheat;
using namespace csgo;

netvars::netvars( ) = default;
netvars::~netvars( ) = default;

void netvars::construct( ) noexcept
{
	this->deps( ).add<csgo_interfaces>( );
	this->deps( ).add<console>( );
}

bool netvars::load( ) noexcept
{
	iterate_client_class(storage_, this->deps( ).get<csgo_interfaces>( ).client->GetAllClasses( ));

	const auto baseent = csgo_modules::client->find_vtable<C_BaseEntity>( );
	iterate_datamap(storage_, baseent->GetDataDescMap( ));
	iterate_datamap(storage_, baseent->GetPredictionDescMap( ));
	store_handmade_netvars(storage_);

#ifdef CHEAT_NETVARS_RESOLVE_TYPE
	auto* const cons = this->deps( ).try_get<console>( );
	const auto log_result = log_netvars(cons, storage_);
	generate_classes(cons, log_result, storage_, lazy_);
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

