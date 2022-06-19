module;

#include <fd/math/internal/fwd.h>
#include <fd/math/internal/vec_fwd.h>

export module fd.math.vector2;

export namespace fd::math
{
    struct vector2
    {
        float x, y;

        vector2(float x, float y);
        vector2(float xy);
        vector2();

        FD_MATH_OP_FWD(vector2);
        FD_MATH_VEC_FWD(vector2);
    };
} // namespace fd::math
