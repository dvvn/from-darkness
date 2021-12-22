module;

#include <cstdint>
#include <limits>
#include <array>
#include <algorithm>
#include <cmath>
#include <ranges>

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

	struct Vector_base_tag { };

	template<size_t Number>
	class Vector_base_item
	{
		std::array<uint8_t, sizeof(float)* (Number == 0 ? 0 : Number - 1)> pad_;
		float val_;

	public:
		constexpr operator const float& ()const { return val_; }
		constexpr operator float& () { return val_; }
	};

	static_assert(sizeof(Vector_base_item<__LINE__>) == sizeof(float) * __LINE__);

	template<typename ...Args>
	Vector_base(Args...)->Vector_base<sizeof...(Args)>;

	template<typename R, class Vb >
	concept Vector_base_op = std::derived_from<Vb, Vector_base_tag> && array_view_constructible<decltype(std::declval<Vb>( )._Data), R>;

#define VECTOR_BASE_OPERATOR(_OP_)\
	template<class Vb, Vector_base_op<Vb> R>\
	constexpr Vb& operator##_OP_##=(Vb& l, R&& r)\
	{\
		l._Data _OP_##= std::forward<R>(r);\
		return l;\
	}\
	template<class Vb, Vector_base_op<Vb> R>\
	constexpr Vb operator##_OP_(Vb l, R&& r)\
	{\
		l._Data _OP_##= std::forward<R>(r);\
		return l;\
	}

	VECTOR_BASE_OPERATOR(+);
	VECTOR_BASE_OPERATOR(-);
	VECTOR_BASE_OPERATOR(*);
	VECTOR_BASE_OPERATOR(/ );

	template<class Vb, Vector_base_op<Vb> R>
	constexpr bool operator==(const Vb& l, R&& r)
	{
		if constexpr (std::ranges::range<R>)
		{
			return std::ranges::equal(l, std::forward<R>(r));
		}
		else
		{
			Vb tmp = std::forward<R>(r);
			return l == std::move(tmp);
		}
	}

	template<class Vb, Vector_base_op<Vb> R>
	constexpr bool operator!=(const Vb& l, R&& r)
	{
		return !(l == std::forward<R>(r));
	}

#define CONST_CLONE(_PRE_,_POST_) \
	_PRE_ const {return _POST_;}\
	_PRE_ {return _POST_; }

#define VECTOR_BASE_IMPL\
	template<typename ...Args>\
	constexpr Vector_base(Args&&...args) : _Data(args...) { }\
	CONST_CLONE(constexpr auto& operator[](size_t idx), _Data[idx] )\
	CONST_CLONE(constexpr auto& operator->( ), _Data)\
	CONST_CLONE(constexpr auto data( ), _Data.data( ))\
	CONST_CLONE(constexpr auto begin( ), _Data.begin( ))\
	CONST_CLONE(constexpr auto end( ), _Data.end( ))\
	constexpr auto size( ) const {return _Data.size();}

#define VECTOR_BASE_FUNCS\
	float Length( ) const\
	{\
		return std::sqrt(LengthSqr( ));\
	}\
	constexpr float LengthSqr( ) const\
	{\
		float tmp = 0;\
		for (auto v : *this)\
			tmp += v * v;\
		return tmp;\
	}\
	float DistTo(const _This_type& other) const\
	{\
		const auto delta = *this - other;\
		return delta.Length( );\
	}\
	float DistToSqr(const _This_type& other) const\
	{\
		const auto delta = *this - other;\
		return delta.LengthSqr( );\
	}\
	_This_type Normalized( ) const\
	{\
		const auto l = Length( );\
		if (l != 0.0f)\
			return *this / l;\
		else\
			return 0.0f;\
	}\
	constexpr float Dot(const _This_type& other) const\
	{\
		float tmp = 0;\
		for (size_t i = 0; i < this->size(); ++i)\
			tmp +=(*this)[i]*other[i];\
		return tmp;\
	}

#define VECTOR_BASE_FUNCS_2D\
	float Length2D( ) const\
	{\
		return std::sqrt(Length2DSqr( ));\
	}\
	constexpr float Length2DSqr( ) const\
	{\
		return (*this)[0] * (*this)[0] + (*this)[1] * (*this)[1];\
	}

	template<>
	struct Vector_base<2> :Vector_base_tag
	{
		union
		{
			array_view<float, 2> _Data;
			Vector_base_item<0> x;
			Vector_base_item<1> y;
		};

		VECTOR_BASE_IMPL;
	};

	template<>
	struct Vector_base<3> :Vector_base_tag
	{
		union
		{
			array_view<float, 3, Vector3_default_value> _Data;
			Vector_base_item<0> x;
			Vector_base_item<1> y;
			Vector_base_item<2> z;
		};

		VECTOR_BASE_IMPL;
	};

	template<>
	struct Vector_base<4> :Vector_base_tag
	{
		union
		{
			array_view<float, 4> _Data;
			Vector_base_item<0> x;
			Vector_base_item<1> y;
			Vector_base_item<2> z;
			Vector_base_item<3> w;
		};

		VECTOR_BASE_IMPL;
	};

	class Vector2D :public array_view<float, 2>
	{
	public:
		using array_view::array_view;

		// Get the vector's magnitude.
		float Length( ) const;

		// Get the vector's magnitude squared.
		float LengthSqr(void) const;

		float    NormalizeInPlace( );
		Vector2D Normalized( ) const;
		bool     IsLengthGreaterThan(float val) const;
		bool     IsLengthLessThan(float val) const;

		// check if a vector is within the box defined by two other vectors
		bool WithinAABox(Vector2D const& boxmin, Vector2D const& boxmax);

		// Get the distance from this vector to the other one.
		float DistTo(const Vector2D& vOther) const;

		// Get the distance from this vector to the other one squared.
		// NJS: note, VC wasn't inlining it correctly in several deeply nested inlines due to being an 'out of line' .  
		// may be able to tidy this up after switching to VC7
		float DistToSqr(const Vector2D& vOther) const;

		// Copy
		void CopyToArray(float* rgfl) const;

		// Multiply, add, and assign to this (ie: *this = a + b * scalar). This
		// is about 12% faster than the actual vector equation (because it's done per-component
		// rather than per-vector).
		void MulAdd(const Vector2D& a, const Vector2D& b, float scalar);

		// Dot product.
		float Dot(const Vector2D& vOther) const;

		// 2d
		float Length2D(void) const;
		float Length2DSqr(void) const;

		/// Get the component of this vector parallel to some other given vector
		Vector2D ProjectOnto(const Vector2D& onto);


		// arithmetic operations
		Vector2D operator-(void) const;

		// Cross product between two vectors.
		Vector2D Cross(const Vector2D& vOther) const;

		// Returns a vector with the min or max in X, Y, and Z.
		Vector2D Min(const Vector2D& vOther) const;
		Vector2D Max(const Vector2D& vOther) const;

		[[no_unique_address]] float x, y;
	};

	class Vector :public Vector_base<3>
	{
	public:
		using _This_type = Vector;

		using Vector_base::Vector_base;
		VECTOR_BASE_FUNCS;
		VECTOR_BASE_FUNCS_2D;
	};

	class alignas(16) VectorAligned : public Vector
	{
	public:
		VectorAligned(const Vector& other = {});

		VectorAligned& operator=(const VectorAligned& other);

		float w;
	};

	class Vector4D :public array_view<float, 4>
	{
	public:

		using array_view::array_view;

		// Get the vector's magnitude.
		float Length( ) const;

		// Get the vector's magnitude squared.
		float LengthSqr( ) const;

		float    NormalizeInPlace( );
		Vector4D Normalized( ) const;

		// check if a vector is within the box defined by two other vectors
		bool WithinAABox(const Vector4D& boxmin, const Vector4D& boxmax);

		// Get the distance from this vector to the other one.
		float DistTo(const Vector4D& vOther) const;

		// Get the distance from this vector to the other one squared.
		// NJS: note, VC wasn't inlining it correctly in several deeply nested inlines due to being an 'out of line' .  
		// may be able to tidy this up after switching to VC7
		float DistToSqr(const Vector4D& vOther) const;

		// Dot product.
		float Dot(const Vector4D& vOther) const;

		// 2d
		float Length2D( ) const;
		float Length2DSqr( ) const;

		/// Get the component of this vector parallel to some other given vector
		Vector4D ProjectOnto(const Vector4D& onto);

		[[no_unique_address]] float x, y, z, w;
	};


}
