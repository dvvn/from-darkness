#pragma once
#include "functional/vfunc.h"

namespace fd::native::inline cs2
{
// Source2EngineToClient001
class engine_client
{
  public:
    bool is_in_game()
    {
        using fn_t = bool (engine_client::*)();
        return vfunc<fn_t>{32, this}();
    }

    int get_local_player()
    {
        int index;
        using fn_t = void (engine_client::*)(int*, int);
        vfunc<fn_t>{44, this}(&index, 0);
        return index + 1;
    }

    int get_engine_build_number()
    {
        using fn_t = int (engine_client::*)();
        return vfunc<fn_t>{76, this}();
    }
};
}