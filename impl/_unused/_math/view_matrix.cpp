module;

#include <fd/math/internal/impl.h>
#include <fd/math/internal/matrix_impl.h>

module fd.math.view_matrix;

using namespace fd::math;

FD_MATH_ARRAY_INIT(view_matrix, float, 4 * 4);
FD_MATH_OP(view_matrix);

void view_matrix::set(const size_t index, const vector4& vec)
{
    data_[0][index] = vec.x;
    data_[1][index] = vec.y;
    data_[2][index] = vec.z;
    data_[3][index] = vec.w;
}

vector4 view_matrix::at(const size_t index) const
{
    return {data_[0][index], data_[1][index], data_[2][index], data_[3][index]};
}

FD_MATH_MATRIX(view_matrix);

view_matrix::operator const matrix3x4&() const
{
    return *reinterpret_cast<const matrix3x4*>(this);
}
