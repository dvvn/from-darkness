#pragma once

#include <fd/valve/app_system.h>
#include <fd/valve/client_class.h>

namespace fd::valve
{
/* // Used by RenderView
enum RenderViewInfo_t
{
    RENDERVIEW_UNSPECIFIED              = 0,
    RENDERVIEW_DRAWVIEWMODEL            = (1 << 0),
    RENDERVIEW_DRAWHUD                  = (1 << 1),
    RENDERVIEW_SUPPRESSMONITORRENDERING = (1 << 2),
}; */

struct base_client
{
    enum frame_stage
    {
        FRAME_UNDEFINED = -1,
        FRAME_START,
        FRAME_NET_UPDATE_START,
        FRAME_NET_UPDATE_POSTDATAUPDATE_START,
        FRAME_NET_UPDATE_POSTDATAUPDATE_END,
        FRAME_NET_UPDATE_END,
        FRAME_RENDER_START,
        FRAME_RENDER_END,
    };

    virtual int Connect(create_interface_fn app_system_factory, /* global_vars_base */ void *globals) = 0;
    virtual int Disconnect()                                                                          = 0;
    virtual int Init(create_interface_fn app_system_factory, /* global_vars_base */ void *globals)    = 0;
    virtual void PostInit()                                                                           = 0;
    virtual void Shutdown()                                                                           = 0;
    virtual void LevelInitPreEntity(const char *map_name)                                             = 0;
    virtual void LevelInitPostEntity()                                                                = 0;
    virtual void LevelShutdown()                                                                      = 0;
    virtual client_class *GetAllClasses()                                                             = 0;

    void CreateMove(int nSequenceNumber, float flInputSampleFrametime, bool bIsActive) {}

    // Notification that we're moving into another stage during the frame.
    // void FrameStageNotify(ClientFrameStage_t stage);

    // The engine has received the specified user message, this code is used to dispatch the message handler
    bool DispatchUserMessage(int msg_type, int flags, int size, const void *msg);
};

} // namespace fd::valve