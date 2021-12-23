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

	class QAngle_base
	{
	public:
		union
		{
			QAngle_base_data _Data;
			QAngle_base_item<0> pitch;
			QAngle_base_item<1> yaw;
			QAngle_base_item<2> roll;
		};

		using value_type = float;

		template<typename ...Args>
		constexpr QAngle_base(Args&&...args) : _Data(args...)
		{
		}
	};

	template<typename R, class Vb>
	concept QAngle_base_op = std::derived_from<Vb, QAngle_base> && array_view_constructible<QAngle_base_data, R>;

#define QANGLE_BASE_OPERATOR(_OP_) ARRAY_VIEW_OPERATOR(_OP_, QAngle_base_op)

	QANGLE_BASE_OPERATOR(+);
	QANGLE_BASE_OPERATOR(-);
	QANGLE_BASE_OPERATOR(*);
	QANGLE_BASE_OPERATOR(/ );
	ARRAY_VIEW_OPERATOR2(QAngle_base_op);

	class QAngle :public QAngle_base
	{
	public:
		using _This_type = QAngle;

		ARRAY_VIEW_DATA_PROXY;
		ARRAY_VIEW_LENGTH;
		ARRAY_VIEW_DIST_TO;
		ARRAY_VIEW_NORMALIZE;

		using QAngle_base::QAngle_base;
	};
}
