module;

#include <fds/math/internal/fwd.h>
#include <fds/math/internal/vec_fwd.h>

export module fds.math.vector3;

export namespace fds::math
{
#if 0
	template<size_t Size>
	struct Vector_base;

	template<size_t Count, array_view_default_value Def = 0.f>
	using Vector_base_data = array_view<float, Count, Def>;

	template<size_t Number>
	using Vector_base_item = Array_view_item<float, Number, Number>;

#define VECTOR_BASE_CONSTRUCTOR           \
    template <typename... Args>           \
    constexpr Vector_base(Args&&... args) \
        : _Data(args...)                  \
    {                                     \
    }

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

	class math::vector2 :public Vector_base_impl<2>
	{
	public:
		using Vector_base_impl::Vector_base_impl;

		using Vector_base_impl::x;
		using Vector_base_impl::y;
	};

	class math::vector3 :public Vector_base_impl<3>
	{
	public:
		using Vector_base_impl::Vector_base_impl;

		constexpr math::vector3 Cross(const math::vector3& vecCross) const
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

		constexpr float Dot(const math::vector3& other) const
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

	class alignas(16) math::vector3_aligned : public math::vector3
	{
	public:
		template<typename ...Args>
		constexpr math::vector3_aligned(Args&&...args) : math::vector3(args...)
		{
		}

	private:
		std::array<uint8_t, sizeof(float)> pad_;
	};

	class math::vector4 :public Vector_base_impl<4>
	{
	public:
		using Vector_base_impl::Vector_base_impl;

		// check if a vector is within the box defined by two other vectors
		constexpr bool WithinAABox(const math::vector4& boxmin, const math::vector4& boxmax)
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

	//-----

#endif

    struct vector3
    {
        float x, y, z;

        vector3(float x, float y, float z);
        vector3(float xyz);
        vector3();

        FDS_MATH_OP_FWD(vector3);
        FDS_MATH_VEC_FWD(vector3);

        vector3 cross(const vector3& other) const;
        float dot(const vector3& other) const;
    };

    struct alignas(16) vector3_aligned : vector3
    {
        vector3_aligned(const vector3& base = {});
    };
} // namespace fds::math
