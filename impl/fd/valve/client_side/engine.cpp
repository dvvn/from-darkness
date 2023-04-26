#include <fd/valve/client_side/engine.h>
#include <fd/vfunc.h>

namespace fd::valve::client_side
{
char const *engine::GetProductVersionString() const
{
    return vtable(this).call<char const *>(105);
}
}