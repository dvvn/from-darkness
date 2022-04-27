module;

#include <cheat/math/internal/vec_impl.h>

#include <limits>

module cheat.math.vector2;

using namespace cheat::math;

vector2::vector2(float x, float y)
	:x(x), y(y)
{
}

vector2::vector2(float xy)
	:vector2(xy, xy)
{
}

vector2::vector2( )
	:vector2(std::numeric_limits<float>::signaling_NaN( ))
{
}

CHEAT_MATH_ARRAY_INIT(vector2, float, 2);
CHEAT_MATH_OP(vector2);
CHEAT_MATH_VEC(vector2);
