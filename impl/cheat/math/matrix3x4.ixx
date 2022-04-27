module;

#include <cheat/math/internal/fwd.h>
#include <cheat/math/internal/matrix_fwd.h>

export module cheat.math.matrix3x4;
export import cheat.math.vector3;

export namespace cheat::math
{
	class matrix3x4
	{
		float data_[3][4];

	public:
		CHEAT_MATH_OP_FWD(matrix3x4);
		CHEAT_MATH_MATRIX_FWD(matrix3x4, vector3);
	};

	struct alignas(16) matrix3x4_aligned :matrix3x4
	{
		matrix3x4_aligned(const matrix3x4& base = {});
	};
}
