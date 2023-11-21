#include "native/engine_client.hpp"
#include "functional/vfunc.h"

namespace fd::native
{
bool engine_client::is_in_game()
{
    using fn_t = bool (engine_client::*)();
    return vfunc<fn_t>{32, this}();
}

int engine_client::get_local_player()
{
    int index;

#ifdef _WIN32
    using fn_t = void (engine_client::*)(int*, int);
#elif defined(__linux__)
    using fn_t = int (CEngineClient::*)(int);
#endif

    vfunc<fn_t> vfunc{44, this};

#ifdef _WIN32
    vfunc(&index, 0);
#elif defined(__linux__)
    index = vfunc(&index);
#endif

    return index + 1;
}

int engine_client::get_engine_build_number()
{
    using fn_t = int (engine_client::*)();
    return vfunc<fn_t>{76, this}();
}
}