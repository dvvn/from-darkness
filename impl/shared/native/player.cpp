#include "player.h"
#include "library_info/native.h"

namespace fd
{
native_player::native_player()
{
    (void)this;
}

native_player::native_player(native_library_info info)
{
    construct_at(this, info.vtable("class C_CSPlayer"));
}

native_player::native_player(void *native)
{
    construct_at(this, native);
}
}