#include <fd/valve/vector3d.h>

namespace fd::valve
{
static_assert(sizeof(vector3d) == sizeof(float[3]));
}