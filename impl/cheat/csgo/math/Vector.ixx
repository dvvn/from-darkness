module;

#include "vector_base_includes.h"

export module cheat.csgo.math.Vector;
export import cheat.csgo.math.vector_base;

namespace cheat::csgo
{
	template<typename Out, typename T, typename T1, size_t ...I>
	constexpr Out _Vector_dot(const T& tpl, const T1& tpl2, std::index_sequence<I...> seq)
	{
		constexpr auto func = [](Out l, Out r) {return l * r; };
		return (std::invoke(func, std::get<I>(tpl), std::get<I>(tpl2)) + ...);
	}

	template<typename T, typename T1, size_t ...I>
	constexpr bool _Within_bbox(const T& t, const T1& min, const T1& max, std::index_sequence<I...> seq)
	{
		constexpr auto check = []<typename Q>(Q v, Q l, Q r)
		{
			return v >= l && v <= r;
		};

		return (std::invoke(check, std::get<I>(t), std::get<I>(min), std::get<I>(max)) && ...);
	}
}

export namespace cheat::csgo
{
	template<size_t Size>
	struct Vector_base;

	template<size_t Count, array_view_default_value Def = 0.f>
	using Vector_base_data = array_view<float, Count, Def>;

	template<size_t Number>
	using Vector_base_item = Array_view_item<float, Number, Number>;

#define VECTOR_BASE_CONSTRUCTOR\
	template<typename ...Args>\
	constexpr Vector_base(Args&&...args) : _Data(args...){ }

	template<>
	struct Vector_base<2>
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
	struct Vector_base<3>
	{
		union
		{
			Vector_base_data<3, std::numeric_limits<float>::infinity( )> _Data;
			Vector_base_item<0> x;
			Vector_base_item<1> y;
			Vector_base_item<2> z;
		};

		VECTOR_BASE_CONSTRUCTOR;
	};

	template<>
	struct Vector_base<4>
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

	template<typename ...Args>
	Vector_base(Args...)->Vector_base<sizeof...(Args)>;

	template<size_t Size, class Base = _Vector_math_base<_Array_view_proxy<Vector_base<Size>>, float>>
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

		constexpr Vector Cross(const Vector& vecCross) const
		{
			auto& vect_A = *this;
			auto& vect_B = vecCross;

			return
			{
			vect_A[1] * vect_B[2] - vect_A[2] * vect_B[1]
			,vect_A[2] * vect_B[0] - vect_A[0] * vect_B[2]
			,vect_A[0] * vect_B[1] - vect_A[1] * vect_B[0]
			};
		}

		constexpr float Dot(const Vector& other) const
		{
			/*float tmp = 0;
			for (size_t i = 0; i < Vector_base_impl::size( ); ++i)
				tmp += (*this)[i] * other[i];
			return _Vector_dot<float>();*/
			const smart_tuple l = _Flatten(*this);
			const auto r = _Flatten(other);

			return _Vector_dot<float>(l, r, std::make_index_sequence<l.size( )>( ));
		}
	};

	class alignas(16) VectorAligned : public Vector
	{
	public:
		template<typename ...Args>
		constexpr VectorAligned(Args&&...args) : Vector(args...)
		{
		}

	private:
		std::array<uint8_t, sizeof(float)> pad_;
	};

	class Vector4D :public Vector_base_impl<4>
	{
	public:
		using Vector_base_impl::Vector_base_impl;

		// check if a vector is within the box defined by two other vectors
		constexpr bool WithinAABox(const Vector4D& boxmin, const Vector4D& boxmax)
		{
			const smart_tuple t = _Flatten(*this);
			const auto min = _Flatten(boxmin);
			const auto max = _Flatten(boxmax);

			return _Within_bbox(t, min, max, std::make_index_sequence<t.size( )>( ));

			/*for (size_t i = 0; i < boxmin.size( ); ++i)
			{
				auto v = (*this)[i];

				if (v < boxmin[i])
					return false;
				if (v > boxmax[i])
					return false;
			}

			return true;*/

			/*return
				_X( ) >= boxmin._X( ) && _X( ) <= boxmax._X( ) &&
				_Y( ) >= boxmin._Y( ) && _Y( ) <= boxmax._Y( ) &&
				_Z( ) >= boxmin._Z( ) && _Z( ) <= boxmax._Z( ) &&
				_W( ) >= boxmin._W( ) && _W( ) <= boxmax._W( );*/
		}
	};
}
