module;

#include <cheat/math/internal/fwd.h>

export module cheat.math.color;

export namespace cheat::math
{
	struct color
	{
		uint8_t r, g, b, a;

		constexpr color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
			:r(r), g(g), b(b), a(a)
		{
		}

		CHEAT_MATH_OP_FWD(color);
	};
}
