module;

#include <fds/math/internal/fwd.h>
#include <fds/math/internal/matrix_fwd.h>

export module fds.math.matrix3x4;
export import fds.math.vector3;

export namespace fds::math
{
    class matrix3x4
    {
        float data_[3][4];

      public:
        FDS_MATH_OP_FWD(matrix3x4);
        FDS_MATH_MATRIX_FWD(matrix3x4, vector3);
    };

    struct alignas(16) matrix3x4_aligned : matrix3x4
    {
        matrix3x4_aligned(const matrix3x4& base = {});
    };
} // namespace fds::math
