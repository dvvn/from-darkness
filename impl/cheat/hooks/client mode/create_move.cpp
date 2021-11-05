#include "create_move.h"

#include "cheat/core/csgo_interfaces.h"

//#include "cheat/players/players_list.h"

#include "cheat/csgo/CUserCmd.hpp"
#include "cheat/csgo/IPrediction.hpp"
#include "cheat/csgo/IVEngineClient.hpp"

using namespace cheat;
using namespace csgo;
using namespace hooks::client_mode;

void create_move_impl::callback(float input_sample_time, CUserCmd* cmd)
{
	const auto original_return = this->call_original_ex(input_sample_time, cmd);

	// is called from CInput::ExtraMouseSample
	if (cmd->command_number == 0)
		return;

	const auto& interfaces = csgo_interfaces::get( );
	return_value_.store_value(false);

	if (original_return == true)
	{
		interfaces->prediction->SetLocalViewAngles(cmd->view_angles);
		interfaces->engine->SetViewAngles(cmd->view_angles);
	}

	if (interfaces->client_state == nullptr || interfaces->engine->IsPlayingDemo( ))
		return;

	bool& send_packet = nstd::address(*this->get_address_of_return_address( ))
					   .remove(4).deref(1)
					   .remove(0x1C).ref( );
}
