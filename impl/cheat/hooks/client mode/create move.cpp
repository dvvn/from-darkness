#include "create move.h"

#include "cheat/core/csgo interfaces.h"
#include "cheat/gui/renderer.h"
#include "cheat/hooks/client/frame stage notify.h"
#include "cheat/players/players list.h"

#include "cheat/sdk/IPrediction.hpp"
#include "cheat/sdk/IVEngineClient.hpp"

using namespace cheat;
using namespace csgo;
using namespace hooks;
using namespace client_mode;
using namespace utl;

create_move::create_move( )
{
	this->Wait_for<players_list>( );

	this->call_original_first_ = true;
}

void create_move::Load( )
{
#ifndef CHEAT_GUI_TEST
	this->target_func_ = method_info::make_member_virtual<ClientModeShared*>(csgo_interfaces::get( ).client_mode, 24);

	this->hook( );
	this->enable( );
#endif
}

void create_move::Callback(float input_sample_time, CUserCmd* cmd)
{
	// is called from CInput::ExtraMouseSample
	if (cmd->command_number == 0)
		return;

	const auto& interfaces = csgo_interfaces::get( );
	const auto  original_return = return_value_.get( );
	return_value_.store_value(false);

	if (original_return == true)
	{
		interfaces.prediction->SetLocalViewAngles(cmd->view_angles);
		interfaces.engine->SetViewAngles(cmd->view_angles);
	}

	if (interfaces.client_state == nullptr || interfaces.engine->IsPlayingDemo( ))
		return;

	auto& send_packet = address(_AddressOfReturnAddress( ))
					   .remove(4).deref(1)
					   .remove(0x1C).ref<bool>( );
}
