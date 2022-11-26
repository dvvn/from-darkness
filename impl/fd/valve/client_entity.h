#pragma once

#include <fd/valve/client_networkable.h>
#include <fd/valve/client_renderable.h>
#include <fd/valve/client_thinkable.h>
#include <fd/valve/client_unknown.h>
#include <fd/valve/matrixX.h>
#include <fd/valve/qangle.h>
#include <fd/valve/vectorX.h>

namespace fd::valve
{
    struct client_entity : client_unknown, client_renderable, client_networkable, client_thinkable
    {
        virtual const vector3& GetAbsOrigin() const                                 = 0;
        virtual const qangle& GetAbsAngles() const                                  = 0;
        virtual void* GetMouth()                                                    = 0;
        virtual bool GetSoundSpatialization(/* SpatializationInfo_t& info */ void*) = 0;
        virtual bool IsBlurred()                                                    = 0;
    };

} // namespace fd::valve
