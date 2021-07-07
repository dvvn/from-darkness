#include "create move.h"

#include "cheat/core/csgo interfaces.h"
#include "cheat/core/netvars.h"
#include "cheat/gui/renderer.h"

#include "cheat/sdk/IPrediction.hpp"
#include "cheat/sdk/IVEngineClient.hpp"

using namespace cheat;
using namespace hooks;
using namespace client_mode;
using namespace utl;

create_move::create_move( )
{
	this->Wait_for<gui::renderer>( );
	this->Wait_for<netvars>( );

	this->call_original_first_ = true;
}

auto create_move::Load( ) -> void
{
#ifndef CHEAT_GUI_TEST
	this->target_func_ = method_info::make_member_virtual(csgo_interfaces::get( ).client_mode.get( ), 24);

	this->hook( );
	this->enable( );
#endif
}

auto create_move::Callback(float input_sample_time, csgo::CUserCmd* cmd) -> void
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

	auto& send_packet = mem::address(_AddressOfReturnAddress( ))
					   .remove(4).deref(1)
					   .remove(0x1C).ref<bool>( );
}
