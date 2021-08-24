#include "create move.h"

#include "cheat/core/csgo interfaces.h"
#include "cheat/hooks/client/frame stage notify.h"
#include "cheat/players/players list.h"

#include "cheat/sdk/IPrediction.hpp"
#include "cheat/sdk/IVEngineClient.hpp"

using namespace cheat;
using namespace csgo;
using namespace hooks::client_mode;

nstd::address create_move::get_target_method_impl( ) const
{
	return dhooks::_Pointer_to_virtual_class_table(csgo_interfaces::get_ptr( )->client_mode.get( ))[24];
}

void create_move::callback(float input_sample_time, CUserCmd* cmd)
{
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

	auto& send_packet = nstd::address(_AddressOfReturnAddress( ))
					   .remove(4).deref(1)
					   .remove(0x1C).ref<bool>( );
}
