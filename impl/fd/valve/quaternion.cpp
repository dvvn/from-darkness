#include <fd/valve/quaternion.h>

namespace fd::valve
{
static_assert(sizeof(quaternion) == sizeof(float[4][4]));
}