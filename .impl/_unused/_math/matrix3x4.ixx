module;

#include <fd/math/internal/fwd.h>
#include <fd/math/internal/matrix_fwd.h>

export module fd.math.matrix3x4;
export import fd.math.vector3;

export namespace fd::math
{
    class matrix3x4
    {
        float data_[3][4];

      public:
        FD_MATH_OP_FWD(matrix3x4);
        FD_MATH_MATRIX_FWD(matrix3x4, vector3);
    };

    struct alignas(16) matrix3x4_aligned : matrix3x4
    {
        matrix3x4_aligned(const matrix3x4& base = {});
    };
} // namespace fd::math
