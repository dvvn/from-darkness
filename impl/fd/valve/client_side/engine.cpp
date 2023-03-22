#include <fd/utils/functional.h>
#include <fd/valve/client_side/engine.h>

namespace fd::valve::client_side
{
char const *engine::GetProductVersionString() const
{
    return vfunc<char const *>(this, 105);
}
}