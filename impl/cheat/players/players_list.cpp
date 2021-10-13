#include "players_list.h"
#include "player.h"

#include "cheat/core/console.h"
#include "cheat/core/csgo interfaces.h"
#include "cheat/core/services loader.h"
#include "cheat/netvars/netvars.h"

#include "cheat/sdk/entity/C_CSPlayer.h"

#include "cheat/sdk/GlobalVars.hpp"
#include "cheat/sdk/IClientEntityList.hpp"
#include "cheat/sdk/IVEngineClient.hpp"
#include "cheat/utils/game.h"

#include <nstd/memory backup.h>
#include <nstd/runtime assert.h>

#include <vector>

using namespace cheat;
using namespace detail;
using namespace csgo;

service_base::load_result players_list::load_impl() noexcept
{
	CHEAT_SERVICE_INIT(CHEAT_FEATURE_PLAYER_LIST)
}

struct players_list::storage_type : std::vector<player>
{
};

players_list::players_list()
{
	storage_ = std::make_unique<storage_type>( );
	this->wait_for_service<netvars>( );
}

players_list::~players_list() = default;

void players_list::update()
{
#if !CHEAT_FEATURE_PLAYER_LIST
	CHEAT_CALL_BLOCKER
#else

	const auto interfaces = csgo_interfaces::get_ptr( );

	const auto max_clients = interfaces->global_vars->max_clients;
	storage_->resize(max_clients);

	const auto curtime = interfaces->global_vars->curtime; //todo: made it fixed
	const auto correct = [&]
	{
		const auto engine = interfaces->engine.get( );
		const auto nci    = engine->GetNetChannelInfo( );

		return std::clamp(nci->GetLatency(FLOW::INCOMING) + nci->GetLatency(FLOW::OUTGOING) + utils::lerp_time( ), 0.f, utils::unlag_limit( ));
	}( );

	for (size_t i = 1; i <= max_clients; ++i)
	{
		auto& entry = (*storage_)[i - 1];
		entry.update(i, curtime, correct);
	}

#endif
}

CHEAT_REGISTER_SERVICE(players_list);
