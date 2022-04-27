module;

#include <cheat/math/internal/impl.h>

#include <limits>

module cheat.math.qangle;

using namespace cheat::math;

qangle::qangle(float pitch, float yaw, float roll)
	:pitch(pitch), yaw(yaw), roll(roll)
{
}

qangle::qangle(float val)
	:qangle(val, val, val)
{
}

qangle::qangle( )
	:qangle(std::numeric_limits<float>::signaling_NaN( ))
{
}

CHEAT_MATH_ARRAY_INIT(qangle, float, 3);
CHEAT_MATH_OP(qangle);
