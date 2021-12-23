module;

#include <limits>
#include <cmath>
#include <ranges>

#include "helpers.h"

export module cheat.csgo.math.Vector;
export import cheat.csgo.math.array_view;

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

		constexpr const float& _X( )const { return _Data[0]; }
		constexpr const float& _Y( )const { return _Data[1]; }

		constexpr  float& _X( ) { return _Data[0]; }
		constexpr  float& _Y( ) { return _Data[1]; }
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

		constexpr const float& _X( )const { return _Data[0]; }
		constexpr const float& _Y( )const { return _Data[1]; }
		constexpr const float& _Z( )const { return _Data[2]; }

		constexpr  float& _X( ) { return _Data[0]; }
		constexpr  float& _Y( ) { return _Data[1]; }
		constexpr  float& _Z( ) { return _Data[2]; }
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

		constexpr const float& _X( )const { return _Data[0]; }
		constexpr const float& _Y( )const { return _Data[1]; }
		constexpr const float& _Z( )const { return _Data[2]; }
		constexpr const float& _W( )const { return _Data[3]; }

		constexpr  float& _X( ) { return _Data[0]; }
		constexpr  float& _Y( ) { return _Data[1]; }
		constexpr  float& _Z( ) { return _Data[2]; }
		constexpr  float& _W( ) { return _Data[3]; }
	};

	template<typename R, class Vb>
	concept Vector_base_op = std::derived_from<Vb, Vector_base_tag> && array_view_constructible<decltype(std::declval<Vb>( )._Data), R>;

#define VECTOR_BASE_OPERATOR(_OP_) ARRAY_VIEW_OPERATOR(_OP_, Vector_base_op)

	VECTOR_BASE_OPERATOR(+);
	VECTOR_BASE_OPERATOR(-);
	VECTOR_BASE_OPERATOR(*);
	VECTOR_BASE_OPERATOR(/ );
	ARRAY_VIEW_OPERATOR2(Vector_base_op);

	template<size_t Size, class Base = Vector_base<Size>>
	struct Vector_base_impl :Base
	{
		template<typename ...Args>
		constexpr Vector_base_impl(Args&&...args) : Base(args...)
		{
		}

		//compiler stuck here
		//using Base::Base;
		using Base::_Data;
		using value_type = float;
		using _This_type = Vector_base_impl;
		using _Base = Base;

		ARRAY_VIEW_DATA_PROXY;
		ARRAY_VIEW_LENGTH;
		ARRAY_VIEW_DIST_TO;
		ARRAY_VIEW_NORMALIZE;
		ARRAY_VIEW_DOT;
	};

	class Vector2D :public Vector_base_impl<2>
	{
	public:
		using Vector_base_impl::Vector_base_impl;
	};

	class Vector :public Vector_base_impl<3>
	{
	public:
		using Vector_base_impl::Vector_base_impl;
		using Vector_base_impl::_This_type;

		constexpr Vector Cross(const _This_type& vecCross) const
		{
			return {
				  _Y( ) * vecCross._Z( ) - _Z( ) * vecCross._Y( )
				, _Z( ) * vecCross._X( ) - _X( ) * vecCross._Z( )
				, _X( ) * vecCross._Y( ) - _Y( ) * vecCross._X( )
			};
		}
	};

	class alignas(16) VectorAligned : public Vector
	{
	public:
		VectorAligned(const Vector& other = {});

		VectorAligned& operator=(const VectorAligned& other);

		float w;
	};

	class Vector4D :public Vector_base_impl<4>
	{
	public:
		using Vector_base_impl::Vector_base_impl;
		using Vector_base_impl::_This_type;

		// check if a vector is within the box defined by two other vectors
		constexpr bool WithinAABox(const _This_type& boxmin, const _This_type& boxmax)
		{
			return
				_X( ) >= boxmin._X( ) && _X( ) <= boxmax._X( ) &&
				_Y( ) >= boxmin._Y( ) && _Y( ) <= boxmax._Y( ) &&
				_Z( ) >= boxmin._Z( ) && _Z( ) <= boxmax._Z( ) &&
				_W( ) >= boxmin._W( ) && _W( ) <= boxmax._W( );
		}
	};
}
