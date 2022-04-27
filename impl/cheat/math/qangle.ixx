module;

#include <cheat/math/internal/fwd.h>

export module cheat.math.qangle;

export namespace cheat::math
{
	struct qangle
	{
		float pitch, yaw, roll;

		qangle(float pitch, float yaw, float roll);
		qangle(float val);
		qangle( );

		CHEAT_MATH_OP_FWD(qangle);
	};
}
