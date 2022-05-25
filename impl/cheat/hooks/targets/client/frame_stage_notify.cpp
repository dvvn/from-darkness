module;

#include <cheat/hooks/hook.h>

module cheat.hooks.client.frame_stage_notify;
import cheat.csgo.interfaces.BaseClient;

using namespace cheat;
using namespace csgo;
using namespace hooks;
using namespace client;

CHEAT_HOOK(frame_stage_notify, member)
{
    frame_stage_notify_impl( )
    {
        this->init({&instance_of<IBaseClientDLL>, 32}, &frame_stage_notify_impl::callback);
    }

    void callback(ClientFrameStage_t stage) const
    {
        switch(stage)
        {
            case FRAME_UNDEFINED: break;
            case FRAME_START: break;
            case FRAME_NET_UPDATE_START: break;
            case FRAME_NET_UPDATE_POSTDATAUPDATE_START: break;
            case FRAME_NET_UPDATE_POSTDATAUPDATE_END:
                //players::update( ); //todo: move to createmove
                break;
            case FRAME_NET_UPDATE_END: break;
            case FRAME_RENDER_START: break;
            case FRAME_RENDER_END: break;
            default:
                //runtime_assert("Unknown frame stage detectetd!");
                break;
        }

        call_original(stage);
    }
};

CHEAT_HOOK_IMPL(frame_stage_notify);
