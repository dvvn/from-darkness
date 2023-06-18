#include "cached_entity.h"

#include <cassert>

namespace fd
{
cached_entity::cached_entity(native_pointer entity)
    : entity_(entity)
{
}

auto cached_entity::native() const -> native_pointer
{
    return entity_;
}

bool cached_entity::valid() const
{
    return entity_ != nullptr;
}

void cached_entity::detach()
{
    assert(entity_ != nullptr);
    entity_ = nullptr;
}

game_entity_index cached_entity::index() const
{
}

} // namespace fd