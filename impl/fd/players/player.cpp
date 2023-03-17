#include <fd/players/player.h>

namespace fd
{
player::player(valve::client_entity *ptr)
    : ptr_(static_cast<valve::cs_player *>(ptr))
{
}

bool player::valid() const
{
    return ptr_ != nullptr;
}

valve::cs_player *player::operator->() const
{
    return ptr_;
}

bool player::operator==(valve::client_entity const *ent) const
{
    return ptr_ == ent;
}

bool player::operator==(player const &other) const
{
    return ptr_ == other.ptr_;
}
}