module;

#include <cheat/math/internal/impl.h>
#include <cheat/math/internal/matrix_impl.h>

module cheat.math.view_matrix;

using namespace cheat::math;

CHEAT_MATH_ARRAY_INIT(view_matrix, float, 4 * 4);
CHEAT_MATH_OP(view_matrix);

void view_matrix::set(const size_t index, const vector4& vec) noexcept
{
	data_[0][index] = vec.x;
	data_[1][index] = vec.y;
	data_[2][index] = vec.z;
	data_[3][index] = vec.w;
}

vector4 view_matrix::at(const size_t index) const noexcept
{
	return {data_[0][index],data_[1][index],data_[2][index],data_[3][index]};
}

CHEAT_MATH_MATRIX(view_matrix);

view_matrix::operator const matrix3x4& () const noexcept
{
	return *reinterpret_cast<const matrix3x4*>(this);
}