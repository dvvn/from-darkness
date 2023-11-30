#include "tier1/functional/vfunc.h"
#include "tier2/native/engine_client.hpp"

namespace FD_TIER2(native, cs2)
{
bool engine_client::is_in_game()
{
    using fn_t = bool (engine_client::*)();
    return vfunc<fn_t>{32, this}();
}

int engine_client::get_local_player()
{
    int index;
    using fn_t = void (engine_client::*)(int*, int);
    vfunc<fn_t>{44, this}(&index, 0);
    return index + 1;
}

int engine_client::get_engine_build_number()
{
    using fn_t = int (engine_client::*)();
    return vfunc<fn_t>{76, this}();
}
}