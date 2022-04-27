module;

#include <cheat/math/internal/vec_impl.h>

#include <limits>

module cheat.math.vector3;

using namespace cheat::math;

vector3::vector3(float x, float y, float z)
	:x(x), y(y), z(z)
{
}

vector3::vector3(float xyz)
	:vector3(xyz, xyz, xyz)
{
}

vector3::vector3( )
	:vector3(std::numeric_limits<float>::signaling_NaN( ))
{
}

CHEAT_MATH_ARRAY_INIT(vector3, float, 3);
CHEAT_MATH_OP(vector3);
CHEAT_MATH_VEC(vector3);

vector3 vector3::cross(const vector3& other) const noexcept
{
	/*auto& vect_A = CHEAT_MATH_AS_ARRAY(*this);
	auto& vect_B = CHEAT_MATH_AS_ARRAY(other);

	return
	{
	 vect_A[1] * vect_B[2] - vect_A[2] * vect_B[1]
	,vect_A[2] * vect_B[0] - vect_A[0] * vect_B[2]
	,vect_A[0] * vect_B[1] - vect_A[1] * vect_B[0]
	};*/

	return
	{
	 y * other.z - z * other.y
	,z * other.x - x * other.z
	,x * other.y - y * other.x
	};
}

float vector3::dot(const vector3& other) const noexcept
{
	return {x * other.x + y * other.y + z * other.z};
}

//-----

vector3_aligned::vector3_aligned(const vector3& base)
	:vector3(base)
{
}