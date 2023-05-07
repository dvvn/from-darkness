#include "client.h"

#include <fd/vfunc.h>

namespace fd::valve
{
auto client_class_range::begin() const -> iterator
{
    return vtable(interface_).call<client_class *>(8);
}

auto client_class_range::end() const -> iterator
{
    (void)this;
    return nullptr;
}
}