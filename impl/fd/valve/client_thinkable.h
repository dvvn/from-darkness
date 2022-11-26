#pragma once

namespace fd::valve
{
    struct client_thinkable
    {
        using handle_type = void*;

        virtual client_unknown* GetIClientUnknown()     = 0;
        virtual void ClientThink()                      = 0;
        virtual handle_type GetThinkHandle()            = 0;
        virtual void SetThinkHandle(handle_type hThink) = 0;
        virtual void Release()                          = 0;
    };
} // namespace fd::valve