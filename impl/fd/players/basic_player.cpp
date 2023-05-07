#include "basic_player.h"

#include <cassert>

namespace fd
{
basic_player::basic_player(void *entity)
    : entity_(entity)
{
}

void *basic_player::native() const
{
    return entity_;
}

bool basic_player::valid() const
{
    return entity_ != nullptr;
}

void basic_player::destroy()
{
    assert(entity_ != nullptr);
    entity_ = nullptr;
}

bool basic_player::operator==(basic_player other) const
{
    return entity_ == other.entity_;
}
} // namespace fd