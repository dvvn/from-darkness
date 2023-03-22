module;

#include <fd/math/internal/fwd.h>
#include <fd/math/internal/matrix_fwd.h>

export module fd.math.view_matrix;
export import fd.math.matrix3x4;
export import fd.math.vector4;

export namespace fd::math
{
    class view_matrix
    {
        float data_[4][4];

      public:
        FD_MATH_OP_FWD(view_matrix);
        FD_MATH_MATRIX_FWD(view_matrix, vector4);

        operator const matrix3x4&() const;
    };
} // namespace fd::math
