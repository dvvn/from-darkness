module;

#include <fds/math/internal/fwd.h>

export module fds.math.qangle;

export namespace fds::math
{
    struct qangle
    {
        float pitch, yaw, roll;

        qangle(float pitch, float yaw, float roll);
        qangle(float val);
        qangle();

        FDS_MATH_OP_FWD(qangle);
    };
} // namespace fds::math
