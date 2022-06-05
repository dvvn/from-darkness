module;

#include <fds/math/internal/impl.h>
#include <fds/math/internal/matrix_impl.h>

module fds.math.matrix3x4;

using namespace fds::math;

FDS_MATH_ARRAY_INIT(matrix3x4, float, 3 * 4);
FDS_MATH_OP(matrix3x4);

void matrix3x4::set(const size_t index, const vector3& vec)
{
    data_[0][index] = vec.x;
    data_[1][index] = vec.y;
    data_[2][index] = vec.z;
}

vector3 matrix3x4::at(const size_t index) const
{
    return {data_[0][index], data_[1][index], data_[2][index]};
}

FDS_MATH_MATRIX(matrix3x4);

//-----

matrix3x4_aligned::matrix3x4_aligned(const matrix3x4& base)
    : matrix3x4(base)
{
}
