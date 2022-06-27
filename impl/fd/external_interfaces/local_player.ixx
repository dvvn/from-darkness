module;

#include <fd/core/object.h>

export module fd.local_player;
export import fd.cs_player;

FD_OBJECT(local_player, fd::cs_player**);

export namespace fd
{
    using ::local_player;
}
