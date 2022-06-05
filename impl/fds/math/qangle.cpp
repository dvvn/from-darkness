module;

#include <fds/math/internal/impl.h>

#include <limits>

module fds.math.qangle;

using namespace fds::math;

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

FDS_MATH_ARRAY_INIT(qangle, float, 3);
FDS_MATH_OP(qangle);
