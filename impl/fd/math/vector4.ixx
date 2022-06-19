module;

#include <fd/math/internal/fwd.h>
#include <fd/math/internal/vec_fwd.h>

export module fd.math.vector4;

export namespace fd::math
{
    struct vector4
    {
        float x, y, z, w;

        vector4(float x, float y, float z, float w);
        vector4(float xyzw);
        vector4();

        FD_MATH_OP_FWD(vector4);
        FD_MATH_VEC_FWD(vector4);

        bool within_aa_box(const vector4& boxmin, const vector4& boxmax) const;
    };

    using quaternion = vector4;
} // namespace fd::math
