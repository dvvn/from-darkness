#include <fd/valve/matrix3x4.h>

namespace fd::valve
{
static_assert(sizeof(matrix3x4) == sizeof(float[3][4]));
}