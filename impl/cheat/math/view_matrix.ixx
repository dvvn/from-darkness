module;

#include <cheat/math/internal/fwd.h>
#include <cheat/math/internal/matrix_fwd.h>


export module cheat.math.view_matrix;
export import cheat.math.matrix3x4;
export import cheat.math.vector4;

export namespace cheat::math
{
	class view_matrix
	{
		float data_[4][4];

	public:
		CHEAT_MATH_OP_FWD(view_matrix);
		CHEAT_MATH_MATRIX_FWD(view_matrix, vector4);

        operator const matrix3x4&() const;
    };
}
