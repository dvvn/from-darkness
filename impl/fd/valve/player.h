#pragma once
#include "interface.h"

namespace fd
{
FD_BIND_NATIVE_INTERFACE(C_CSPlayer, client);

union native_player
{
    FD_NATIVE_INTERFACE(C_CSPlayer);
};
}