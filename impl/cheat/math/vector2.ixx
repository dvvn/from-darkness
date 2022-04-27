module;

#include <cheat/math/internal/fwd.h>
#include <cheat/math/internal/vec_fwd.h>

export module cheat.math.vector2;

export namespace cheat::math
{
	struct vector2
	{
		float x, y;

		vector2(float x, float y);
		vector2(float xy);
		vector2( );

		CHEAT_MATH_OP_FWD(vector2);
		CHEAT_MATH_VEC_FWD(vector2);
	};
}
