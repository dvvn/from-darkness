module;

#include <limits>
#include <cmath>
#include <ranges>

#include "helpers.h"

export module cheat.csgo.math.Vector;
export import cheat.csgo.math.array_view;

export using cheat::csgo::_Array_view_proxy;
export using cheat::csgo::_Array_view_proxy_math;

export namespace cheat::csgo
{
	struct Vector3_default_value
	{
		constexpr float operator()( )const
		{
			return std::numeric_limits<float>::infinity( );
		}
	};

	template<size_t Size>
	struct Vector_base;

	struct Vector_base_tag
	{
	};

	template<typename ...Args>
	Vector_base(Args...)->Vector_base<sizeof...(Args)>;

	template<size_t Count, typename ...Def>
	using Vector_base_data = array_view<float, Count, Def...>;

	template<size_t Number>
	using Vector_base_item = Array_view_item<Number, float>;

#define VECTOR_BASE_CONSTRUCTOR\
	template<typename ...Args>\
	constexpr Vector_base(Args&&...args) : _Data(args...){ }

	template<>
	struct Vector_base<2> :Vector_base_tag
	{
		union
		{
			Vector_base_data<2> _Data;
			Vector_base_item<0> x;
			Vector_base_item<1> y;
		};

		VECTOR_BASE_CONSTRUCTOR;
	};

	template<>
	struct Vector_base<3> :Vector_base_tag
	{
		union
		{
			Vector_base_data<3, Vector3_default_value> _Data;
			Vector_base_item<0> x;
			Vector_base_item<1> y;
			Vector_base_item<2> z;
		};

		VECTOR_BASE_CONSTRUCTOR;
	};

	template<>
	struct Vector_base<4> :Vector_base_tag
	{
		union
		{
			Vector_base_data<4> _Data;
			Vector_base_item<0> x;
			Vector_base_item<1> y;
			Vector_base_item<2> z;
			Vector_base_item<3> w;
		};

		VECTOR_BASE_CONSTRUCTOR;
	};

	template<typename R, class Vb>
	concept Vector_base_op = std::derived_from<Vb, Vector_base_tag> && array_view_constructible<decltype(std::declval<Vb>( )._Data), R>;

	ARRAY_VIEW_OPERATORS(Vector_base_op);

	template<size_t Size, class Base = _Array_view_proxy_math<_Array_view_proxy<Vector_base<Size>>, float>>
	struct Vector_base_impl : Base
	{
		template<typename ...Args>
		constexpr Vector_base_impl(Args&&...args) : Base(args...)
		{
		}

		//using Base::Base; compiler stuck here
	};

	class Vector2D :public Vector_base_impl<2>
	{
	public:
		using Vector_base_impl::Vector_base_impl;

		using Vector_base_impl::x;
		using Vector_base_impl::y;
	};

	class Vector :public Vector_base_impl<3>
	{
	public:
		using Vector_base_impl::Vector_base_impl;

		using Vector_base_impl::x;
		using Vector_base_impl::y;
		using Vector_base_impl::z;

		constexpr Vector Cross(const Vector& vecCross) const
		{
			Vector cross_P;
			auto& vect_A = *this;
			auto& vect_B = vecCross;

			cross_P[0] = vect_A[1] * vect_B[2] - vect_A[2] * vect_B[1];
			cross_P[1] = vect_A[2] * vect_B[0] - vect_A[0] * vect_B[2];
			cross_P[2] = vect_A[0] * vect_B[1] - vect_A[1] * vect_B[0];

			return cross_P;
		}

		constexpr float Dot(const Vector& other) const
		{
			float tmp = 0;
			for (size_t i = 0; i < Vector_base_impl::size( ); ++i)
				tmp += (*this)[i] * other[i];
			return tmp;
		}
	};

	class alignas(16) VectorAligned : public Vector
	{
	public:
		using Vector::Vector_base_impl;

		float w;
	};

	class Vector4D :public Vector_base_impl<4>
	{
	public:
		using Vector_base_impl::Vector_base_impl;

		using Vector_base_impl::x;
		using Vector_base_impl::y;
		using Vector_base_impl::z;
		using Vector_base_impl::w;

		// check if a vector is within the box defined by two other vectors
		constexpr bool WithinAABox(const Vector4D& boxmin, const Vector4D& boxmax)
		{
			for (size_t i = 0; i < boxmin.size( ); ++i)
			{
				auto v = (*this)[i];

				if (v < boxmin[i])
					return false;
				if (v > boxmax[i])
					return false;
			}

			return true;

			/*return
				_X( ) >= boxmin._X( ) && _X( ) <= boxmax._X( ) &&
				_Y( ) >= boxmin._Y( ) && _Y( ) <= boxmax._Y( ) &&
				_Z( ) >= boxmin._Z( ) && _Z( ) <= boxmax._Z( ) &&
				_W( ) >= boxmin._W( ) && _W( ) <= boxmax._W( );*/
		}
	};
}
