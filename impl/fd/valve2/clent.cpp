#include "client.h"

#include <fd/vfunc.h>

namespace fd::valve
{
auto client_class_range::begin() const -> iterator
{
    return vtable(interface_)[8](return_type<client_class *>);
}

auto client_class_range::end() const -> iterator
{
    (void)this;
    return nullptr;
}
}