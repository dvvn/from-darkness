module;

#include <fd/math/internal/fwd.h>

export module fd.math.qangle;

export namespace fd::math
{
    struct qangle
    {
        float pitch, yaw, roll;

        qangle(float pitch, float yaw, float roll);
        qangle(float val);
        qangle();

        FD_MATH_OP_FWD(qangle);
    };
} // namespace fd::math
