#include "entity.h"
#include "diagnostics/fatal.h"

#include <cassert>

namespace fd
{
entity::entity(native_pointer entity)
    : entity_(entity)
{
}

auto entity::native() const -> native_pointer
{
    return entity_;
}

bool entity::valid() const
{
    return entity_ != nullptr;
}

void entity::detach()
{
    assert(entity_ != nullptr);
    entity_ = nullptr;
}

bool entity::keep() const
{
    // RESERVED
    return true;
}

native_entity_index entity::index() const
{
    // not implemented
    unreachable();
}

} // namespace fd