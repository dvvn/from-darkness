#include "create move.h"

#include "cheat/core/console.h"
#include "cheat/core/services loader.h"
#include "cheat/core/csgo interfaces.h"

#include "cheat/netvars/config.h"

#include "cheat/players/players_list.h"

#include "cheat/sdk/CUserCmd.hpp"
#include "cheat/sdk/IPrediction.hpp"
#include "cheat/sdk/IVEngineClient.hpp"

using namespace cheat;
using namespace csgo;
using namespace hooks::client_mode;

create_move::create_move()
{
	this->wait_for_service<players_list>( );
	this->get_address_of_return_address( ).emplace( );
}

CHEAT_HOOK_PROXY_INIT_FN(create_move, CHEAT_MODE_INGAME)
CHEAT_HOOK_PROXY_TARGET_FN(create_move, CHEAT_MODE_INGAME, &csgo_interfaces::client_mode, 24);

void create_move::callback(float input_sample_time, CUserCmd* cmd)
{
#if !CHEAT_MODE_INGAME
	CHEAT_CALL_BLOCKER
#else
	const auto original_return = this->call_original_ex(input_sample_time, cmd);

	// is called from CInput::ExtraMouseSample
	if (cmd->command_number == 0)
		return;

	const auto interfaces = csgo_interfaces::get_ptr( );
	return_value_.store_value(false);

	if (original_return == true)
	{
		interfaces->prediction->SetLocalViewAngles(cmd->view_angles);
		interfaces->engine->SetViewAngles(cmd->view_angles);
	}

	if (interfaces->client_state == nullptr || interfaces->engine->IsPlayingDemo( ))
		return;

	auto& send_packet = nstd::address(*this->get_address_of_return_address( ))
					   .remove(4).deref(1)
					   .remove(0x1C).ref<bool>( );
#endif
}

CHEAT_REGISTER_SERVICE(create_move);
