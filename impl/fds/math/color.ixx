module;

#include <fds/math/internal/fwd.h>

export module fds.math.color;

export namespace fds::math
{
    struct color
    {
        uint8_t r, g, b, a;

        constexpr color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
            : r(r)
            , g(g)
            , b(b)
            , a(a)
        {
        }

        FDS_MATH_OP_FWD(color);
    };
} // namespace fds::math
