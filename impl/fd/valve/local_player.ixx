module;

#include <fd/object.h>

export module fd.valve.local_player;
export import fd.valve.cs_player;

using namespace fd::valve;

FD_OBJECT(local_player, cs_player**);

export namespace fd::valve
{
    using ::local_player;
}
