module;

#include <fds/math/internal/vec_impl.h>

#include <limits>

module fds.math.vector2;

using namespace fds::math;

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

FDS_MATH_ARRAY_INIT(vector2, float, 2);
FDS_MATH_OP(vector2);
FDS_MATH_VEC(vector2);
