module;

#include <cheat/hooks/hook.h>

module cheat.hooks.client_mode.create_move;
import cheat.csgo.interfaces.Prediction;
import cheat.csgo.interfaces.EngineClient;
import cheat.csgo.interfaces.ClientMode;

using namespace cheat;
using namespace csgo;
using namespace hooks;
using namespace client_mode;

CHEAT_HOOK(create_move, member)
{
    create_move_impl( )
    {
        this->init({&instance_of<ClientModeShared>, 24}, &create_move_impl::callback);
    }

    bool callback(float input_sample_time, CUserCmd * cmd) const noexcept
    {
        const auto original_return = call_original(input_sample_time, cmd);

        // is called from CInput::ExtraMouseSample
        if(cmd->iCommandNumber == 0)
            return original_return;

        if(original_return == true)
        {
            instance_of<IPrediction>->SetLocalViewAngles(cmd->angViewPoint);
            instance_of<IVEngineClient>->SetViewAngles(cmd->angViewPoint);
        }

        if(/*interfaces.client_state == nullptr ||*/ instance_of<IVEngineClient>->IsPlayingDemo( ))
            return original_return;

        //bool& send_packet = address(/*this->return_address( )*/*this->addr1).remove(4).deref(1).remove(0x1C).ref( );

        return false;
    }
};

CHEAT_HOOK_IMPL(create_move);