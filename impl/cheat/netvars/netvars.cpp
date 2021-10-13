#include "netvars.h"
#include "config.h"
#include "data_dumper.h"
#include "data_filler.h"

#include "cheat/core/service.h"
#include "cheat/core/console.h"
#include "cheat/core/csgo interfaces.h"
#include "cheat/core/csgo modules.h"
#include "cheat/core/services loader.h"

#include "cheat/sdk/IBaseClientDll.hpp"
#include "cheat/sdk/entity/C_BaseEntity.h"

using namespace cheat;
using namespace csgo;

netvars::netvars()
{
	data_ = std::make_unique<detail::netvars_data>( );
}

netvars::~netvars() = default;

service_base::load_result netvars::load_impl() noexcept
{
#if defined(CHEAT_NETVARS_DUMPER_DISABLED)
	CHEAT_SERVICE_SKIPPED
#else

	auto& data = data_->storage;

#ifdef _DEBUG
	data.clear( );
#endif

	const auto interfaces = csgo_interfaces::get_ptr( );

	_Iterate_client_class(data, interfaces->client->GetAllClasses( ));

	const auto baseent = CHEAT_FIND_VTABLE(client, C_BaseEntity);

	_Iterate_datamap(data, baseent->GetDataDescMap( ));
	_Iterate_datamap(data, baseent->GetPredictionDescMap( ));

#if defined(CHEAT_NETVARS_RESOLVE_TYPE)

	const auto info = _Dump_netvars(data_->storage);
	_Generate_classes(info, data_->storage, data_->lazy);

#endif
	CHEAT_SERVICE_LOADED

#endif
}

int netvars::at(const std::string_view& table, const std::string_view& item) const
{
	for (auto& [table_stored, keys] : data_->storage.items( ))
	{
		if (table_stored != table)
			continue;

		for (const auto& [item_stored, data] : keys.items( ))
		{
			if (item_stored == item)
				return data["offset"].get<int>( );
		}

		runtime_assert(false, std::format(__FUNCTION__": item {} not found in table {}", item, table).c_str( ));
		return 0;
	}

	runtime_assert(false, std::format(__FUNCTION__": table {} not found", table).c_str( ));
	return 0;
}

CHEAT_REGISTER_SERVICE(netvars);
