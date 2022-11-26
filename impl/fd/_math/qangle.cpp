module;

#include <fd/math/internal/impl.h>

#include <limits>

module fd.math.qangle;

using namespace fd::math;

qangle::qangle(float pitch, float yaw, float roll)
    : pitch(pitch)
    , yaw(yaw)
    , roll(roll)
{
}

qangle::qangle(float val)
    : qangle(val, val, val)
{
}

qangle::qangle()
    : qangle(std::numeric_limits<float>::signaling_NaN())
{
}

FD_MATH_ARRAY_INIT(qangle, float, 3);
FD_MATH_OP(qangle);
