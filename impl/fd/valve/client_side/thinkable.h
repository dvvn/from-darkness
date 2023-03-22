#pragma once

namespace fd::valve::client_side
{
using think_handle = void *;

struct unknown;

struct thinkable
{
    virtual unknown *GetIClientUnknown()             = 0;
    virtual void ClientThink()                       = 0;
    virtual think_handle GetThinkHandle()            = 0;
    virtual void SetThinkHandle(think_handle hThink) = 0;
    virtual void Release()                           = 0;
};
} // namespace fd::valve::client_side