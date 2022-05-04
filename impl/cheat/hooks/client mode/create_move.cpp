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

CHEAT_HOOK_INSTANCE(client_mode, create_move);

static void* target( ) noexcept
{
	const nstd::mem::basic_address<void> vtable_holder = &nstd::instance_of<ClientModeShared*>;
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
			nstd::instance_of<IPrediction*>->SetLocalViewAngles(cmd->angViewPoint);
			nstd::instance_of<IVEngineClient*>->SetViewAngles(cmd->angViewPoint);
		}

		if (/*interfaces.client_state == nullptr ||*/ nstd::instance_of<IVEngineClient*>->IsPlayingDemo( ))
			return original_return;

		//bool& send_packet = address(/*this->return_address( )*/*this->addr1).remove(4).deref(1).remove(0x1C).ref( );

		return false;
	}
};

CHEAT_HOOK_INIT(client_mode, create_move);
