module;

#include <fds/math/internal/fwd.h>
#include <fds/math/internal/vec_fwd.h>

export module fds.math.vector4;

export namespace fds::math
{
    struct vector4
    {
        float x, y, z, w;

        vector4(float x, float y, float z, float w);
        vector4(float xyzw);
        vector4();

        FDS_MATH_OP_FWD(vector4);
        FDS_MATH_VEC_FWD(vector4);

        bool within_aa_box(const vector4& boxmin, const vector4& boxmax) const;
    };

    using quaternion = vector4;
} // namespace fds::math
