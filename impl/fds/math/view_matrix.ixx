module;

#include <fds/math/internal/fwd.h>
#include <fds/math/internal/matrix_fwd.h>

export module fds.math.view_matrix;
export import fds.math.matrix3x4;
export import fds.math.vector4;

export namespace fds::math
{
    class view_matrix
    {
        float data_[4][4];

      public:
        FDS_MATH_OP_FWD(view_matrix);
        FDS_MATH_MATRIX_FWD(view_matrix, vector4);

        operator const matrix3x4&() const;
    };
} // namespace fds::math
