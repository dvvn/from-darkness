module;

#include "cheat/hooks/base_includes.h"

module cheat.hooks.client_mode:create_move;
import cheat.players;
import nstd.mem.address;
import cheat.csgo.interfaces.Prediction;
import cheat.csgo.interfaces.EngineClient;

using namespace cheat;
using namespace csgo;
using namespace hooks::client_mode;

create_move::create_move( ) = default;

void create_move::construct( ) noexcept
{
	//this->set_target_method(this->deps( ).get<csgo_interfaces>( ).client_mode.vfunc(24));
	const nstd::mem::basic_address vtable_holder = ClientModeShared::get_ptr( );
	this->set_target_method(vtable_holder.deref<1>( )[24]);
	//this->addr1.emplace( );
}

void create_move::callback(float input_sample_time, CUserCmd * cmd)
{
	const auto original_return = this->call_original_and_store_result(input_sample_time, cmd);

	// is called from CInput::ExtraMouseSample
	if (cmd->iCommandNumber == 0)
		return;

	this->store_return_value(false);

	if (original_return == true)
	{
		IPrediction::get( ).SetLocalViewAngles(cmd->angViewPoint);
		IVEngineClient::get( ).SetViewAngles(cmd->angViewPoint);
	}

	if (/*interfaces.client_state == nullptr ||*/ IVEngineClient::get( ).IsPlayingDemo( ))
		return;

	//bool& send_packet = address(/*this->return_address( )*/*this->addr1).remove(4).deref(1).remove(0x1C).ref( );
}
