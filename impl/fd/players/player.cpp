#include "player.h"

namespace fd
{
player::player(entity *ptr)
    : game_ptr_((ptr))
{
}

bool player::valid() const
{
    return game_ptr_ != nullptr;
}

void player::destroy()
{
    assert(game_ptr_ != nullptr);
    game_ptr_ = nullptr;
}

bool player::operator==(entity const &other) const
{
    return game_ptr_ == &other;
}

bool player::operator==(player const &other) const
{
    return game_ptr_ == other.game_ptr_;
}
}