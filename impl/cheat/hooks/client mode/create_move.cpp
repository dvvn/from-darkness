module;

#include <cheat/hooks/instance.h>

module cheat.hooks.client_mode.create_move;
import cheat.players;
import cheat.csgo.interfaces.Prediction;
import cheat.csgo.interfaces.EngineClient;
import cheat.csgo.interfaces.ClientMode;
import cheat.hooks.base;
import nstd.mem.address;

using namespace cheat;
using namespace csgo;
using namespace hooks;

#if 0
using create_move_base = hooks::base<decltype(&IClientMode::CreateMove)>;
struct create_move_impl :create_move_base
{
	create_move_impl( )
	{
		//this->set_target_method(this->deps( ).get<csgo_interfaces>( ).client_mode.vfunc(24));
		const nstd::mem::basic_address vtable_holder = ClientModeShared::get_ptr( );
		this->set_target_method(vtable_holder.deref<1>( )[24]);
		//this->addr1.emplace( );
	}

	void callback(float input_sample_time, CUserCmd* cmd)
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

};
#endif

CHEAT_HOOK_INSTANCE(client_mode, create_move);

static void* target( ) noexcept
{
	const nstd::mem::basic_address vtable_holder = ClientModeShared::get_ptr( );
	return vtable_holder.deref<1>( )[24];
}

struct replace
{
	bool fn(float input_sample_time, CUserCmd* cmd) noexcept
	{
		const auto original_return = CHEAT_HOOK_CALL_ORIGINAL_MEMBER(input_sample_time, cmd);

		// is called from CInput::ExtraMouseSample
		if (cmd->iCommandNumber == 0)
			return original_return;

		if (original_return == true)
		{
			IPrediction::get( ).SetLocalViewAngles(cmd->angViewPoint);
			IVEngineClient::get( ).SetViewAngles(cmd->angViewPoint);
		}

		if (/*interfaces.client_state == nullptr ||*/ IVEngineClient::get( ).IsPlayingDemo( ))
			return original_return;

		//bool& send_packet = address(/*this->return_address( )*/*this->addr1).remove(4).deref(1).remove(0x1C).ref( );

		return false;
	}
};

CHEAT_HOOK_INIT(client_mode, create_move);
