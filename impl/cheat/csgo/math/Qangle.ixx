module;

#include "vector_base_includes.h"

export module cheat.csgo.math.Qangle;
export import cheat.csgo.math.vector_base;

export namespace cheat::csgo
{
	using QAngle_base_data = array_view<float, 3, 0.f>;

	template<size_t Number>
	using QAngle_base_item = Array_view_item<float, Number, Number>;

	struct QAngle_base
	{
		union
		{
			QAngle_base_data _Data;
			QAngle_base_item<0> pitch;
			QAngle_base_item<1> yaw;
			QAngle_base_item<2> roll;
		};

		template<typename ...Args>
		constexpr QAngle_base(Args&&...args) : _Data(args...)
		{
		}
	};

	class QAngle :public _Vector_math_base<_Array_view_proxy<QAngle_base>, float>
	{
	public:

		//using _Vector_math_base::_Vector_math_base; compiler stuck here

		template<typename ...Args>
		constexpr QAngle(Args&&...args) : _Vector_math_base(args...)
		{
		}
	};

	static_assert(sizeof(QAngle) == sizeof(float) * 3);
}
