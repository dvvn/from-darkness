module;

#include <fd/math/internal/vec_impl.h>

#include <limits>

module fd.math.vector2;

using namespace fd::math;

vector2::vector2(float x, float y)
    : x(x)
    , y(y)
{
}

vector2::vector2(float xy)
    : vector2(xy, xy)
{
}

vector2::vector2()
    : vector2(std::numeric_limits<float>::signaling_NaN())
{
}

FD_MATH_ARRAY_INIT(vector2, float, 2);
FD_MATH_OP(vector2);
FD_MATH_VEC(vector2);
