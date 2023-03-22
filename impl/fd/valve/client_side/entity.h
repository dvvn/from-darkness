#pragma once

#include <fd/valve/client_side/networkable.h>
#include <fd/valve/client_side/renderable.h>
#include <fd/valve/client_side/thinkable.h>
#include <fd/valve/client_side/unknown.h>

#include <fd/valve/qangle.h>
#include <fd/valve/vector3d.h>

namespace fd::valve::client_side
{
struct entity : unknown, renderable, networkable, thinkable
{
    virtual vector3d const &GetAbsOrigin() const                           = 0;
    virtual qangle const &GetAbsAngles() const                             = 0;
    virtual void *GetMouth()                                               = 0;
    virtual bool GetSoundSpatialization(struct SpatializationInfo_t &info) = 0;
    virtual bool IsBlurred()                                               = 0;
};
}