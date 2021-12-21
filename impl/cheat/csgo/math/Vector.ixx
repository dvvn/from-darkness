module;

#include <cstdint>
#include <limits>
#include <array>

export module cheat.csgo.math.Vector;
export import cheat.csgo.math.array_view;

export namespace cheat::csgo
{
	struct Vector_default_value
	{
		constexpr float operator()( )const
		{
			return std::numeric_limits<float>::infinity( );
		}
	};



	class Vector :public array_view<float, 4, Vector_default_value>
	{
	public:
		using array_view::array_view;

		void NormalizeInPlace( );
		Vector Normalized( ) const;
		float DistTo(const Vector& other) const;
		float DistToSqr(const Vector& other) const;
		float Dot(const Vector& other) const;

		float Length( ) const;
		float LengthSqr( ) const;
		float Length2D( ) const;

		[[no_unique_address]] float x, y, z;
	};

	class alignas(uint16_t) VectorAligned : public Vector
	{
	public:
		VectorAligned(const Vector& other = {});

		VectorAligned& operator=(const VectorAligned& other);

		float w;
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
