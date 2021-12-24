module;

#include <cmath>
#include "helpers.h"

export module cheat.csgo.math.Qangle;
export import cheat.csgo.math.array_view;

export namespace cheat::csgo
{
	using QAngle_base_data = array_view<float, 3>;

	template<size_t Number>
	using QAngle_base_item = Array_view_item<Number, float>;

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

	struct QAngle_base_impl : _Array_view_proxy_math<_Array_view_proxy<QAngle_base>, float>
	{
		//using _Array_view_proxy_math::_Array_view_proxy_math; compiler stuck here

		template<typename ...Args>
		constexpr QAngle_base_impl(Args&&...args) : _Array_view_proxy_math(args...)
		{
		}
	};

	template<typename R, class Vb>
	concept QAngle_base_op = std::derived_from<Vb, QAngle_base> && array_view_constructible<QAngle_base_data, R>;

	ARRAY_VIEW_OPERATORS(QAngle_base_op);

	class QAngle :public QAngle_base_impl
	{
	public:
		using QAngle_base_impl::QAngle_base_impl;
	};
}
