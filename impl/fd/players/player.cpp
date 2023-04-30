#include <fd/players/player.h>

namespace fd
{
player::player(entity *ptr)
    : ptr_(static_cast<cs_player *>(ptr))
{
}

bool player::valid() const
{
    return ptr_ != nullptr;
}

void player::destroy()
{
    assert(ptr_ != nullptr);
    ptr_ = nullptr;
}

auto player::operator->() const -> cs_player *
{
    return ptr_;
}

bool player::operator==(entity const *ent) const
{
    return ptr_ == ent;
}

bool player::operator==(player const &other) const
{
    return ptr_ == other.ptr_;
}
}