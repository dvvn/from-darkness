#include <fd/valve/matrix4x4.h>

namespace fd::valve
{
static_assert(sizeof(matrix4x4) == sizeof(float[4][4]));
}