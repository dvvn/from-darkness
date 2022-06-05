module;

#include <fds/math/internal/fwd.h>
#include <fds/math/internal/vec_fwd.h>

export module fds.math.vector2;

export namespace fds::math
{
    struct vector2
    {
        float x, y;

        vector2(float x, float y);
        vector2(float xy);
        vector2();

        FDS_MATH_OP_FWD(vector2);
        FDS_MATH_VEC_FWD(vector2);
    };
} // namespace fds::math
