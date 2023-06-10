#include "client.h"

#include <fd/vtable.h>

namespace fd::valve
{
auto client_class_range::begin() const -> iterator
{
    return invoke<client_class *>(vtable(interface_)[8]);
}

auto client_class_range::end() const -> iterator
{
    (void)this;
    return nullptr;
}
}