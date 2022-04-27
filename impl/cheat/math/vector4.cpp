module;

#include <cheat/math/internal/vec_impl.h>

#include <limits>

module cheat.math.vector4;

using namespace cheat::math;

vector4::vector4(float x, float y, float z, float w)
	:x(x), y(y), z(z), w(w)
{
}

vector4::vector4(float xyzw)
	:vector4(xyzw, xyzw, xyzw, xyzw)
{
}

vector4::vector4( )
	:vector4(std::numeric_limits<float>::signaling_NaN( ))
{
}

CHEAT_MATH_ARRAY_INIT(vector4, float, 4);
CHEAT_MATH_OP(vector4);
CHEAT_MATH_VEC(vector4);

bool vector4::within_aa_box(const vector4& boxmin, const vector4& boxmax) const noexcept
{
	return
		(x >= boxmin.x) && (x <= boxmax.x) &&
		(y >= boxmin.y) && (y <= boxmax.y) &&
		(z >= boxmin.z) && (z <= boxmax.z) &&
		(w >= boxmin.w) && (w <= boxmax.w);
}