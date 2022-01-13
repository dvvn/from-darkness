module;

#include "cheat/hooks/base_includes.h"

module cheat.hooks.client_mode:create_move;
import cheat.players;

using namespace cheat;
using namespace csgo;
using namespace hooks::client_mode;
using namespace nstd::mem;

create_move::create_move( ) = default;

void create_move::load_async( ) noexcept
{
	this->addr1.emplace( );
	this->deps( ).add<players_list>( );
}

void* create_move::get_target_method( ) const
{
	return services_loader::get( ).deps( ).get<csgo_interfaces>( ).client_mode.vfunc(24).ptr( );
}

void create_move::callback(float input_sample_time, CUserCmd * cmd)
{
	const auto original_return = this->call_original_and_store_result(input_sample_time, cmd);

	// is called from CInput::ExtraMouseSample
	if (cmd->iCommandNumber == 0)
		return;

	const auto& interfaces = services_loader::get( ).deps( ).get<csgo_interfaces>( );
	this->store_return_value(false);

	if (original_return == true)
	{
		interfaces.prediction->SetLocalViewAngles(cmd->angViewPoint);
		interfaces.engine->SetViewAngles(cmd->angViewPoint);
	}

	if (interfaces.client_state == nullptr || interfaces.engine->IsPlayingDemo( ))
		return;

	bool& send_packet = address(/*this->return_address( )*/*this->addr1).remove(4).deref(1).remove(0x1C).ref( );
}
