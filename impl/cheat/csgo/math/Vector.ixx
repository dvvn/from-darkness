module;

#include <cstdint>
#include <limits>

export module cheat.csgo.math.Vector;

export namespace cheat::csgo
{
	class Vector
	{
	public:
		Vector(float val = std::numeric_limits<float>::infinity( ));
		Vector(float X, float Y, float Z);

		bool IsValid( ) const;

		void Invalidate( );

		float& operator[](int i);
		float operator[](int i) const;

		void NormalizeInPlace( );
		Vector Normalized( ) const;
		float DistTo(const Vector& other) const;
		float DistToSqr(const Vector& other) const;
		float Dot(const Vector& other) const;

		float Length( ) const;
		float LengthSqr( ) const;
		float Length2D( ) const;
		Vector operator-( ) const;

		Vector& operator+=(const Vector& v);
		Vector& operator-=(const Vector& v);
		Vector& operator*=(const Vector& v);
		Vector& operator/=(const Vector& v);

		Vector operator+(const Vector& v) const;
		Vector operator-(const Vector& v) const;
		Vector operator*(const Vector& v) const;
		Vector operator/(const Vector& v) const;

		bool operator==(const Vector& v) const;
		bool operator!=(const Vector& v) const;

		float x, y, z;
	};

	class alignas(uint16_t) VectorAligned : public Vector
	{
	public:
		VectorAligned(const Vector& other = {});

		VectorAligned& operator=(const VectorAligned& other);

		float w;
	};

	class Vector2D
	{
	public:
		// Members
		float x, y;

		// Construction/destruction:
		Vector2D( ) = default;
		Vector2D(float X, float Y);
		Vector2D(float* clr);

		Vector2D(const Vector2D& vOther);

		// Initialization
		void Init(float ix = 0.0f, float iy = 0.0f);
		// TODO (Ilya): Should there be an init that takes a single float for consistency?

		// Got any nasty NAN's?
		bool IsValid( ) const;
		void Invalidate( );

		// array access...
		float  operator[](int i) const;
		float& operator[](int i);

		// Base address...
		float* Base( );
		float const* Base( ) const;

		// Initialization methods
		[[deprecated]]
		void Random(float minVal, float maxVal);
		void Zero( ); ///< zero out a vector

		// equality
		bool operator==(const Vector2D& v) const;
		bool operator!=(const Vector2D& v) const;

		// arithmetic operations
		Vector2D& operator+=(const Vector2D& v);

		Vector2D& operator-=(const Vector2D& v);

		Vector2D& operator*=(float fl);

		Vector2D& operator*=(const Vector2D& v);

		Vector2D& operator/=(const Vector2D& v);

		// this ought to be an opcode.
		Vector2D& operator+=(float fl);

		// this ought to be an opcode.
		Vector2D& operator/=(float fl);

		Vector2D& operator-=(float fl);

		// negate the vector components
		void Negate( );

		// Get the vector's magnitude.
		float Length( ) const;

		// Get the vector's magnitude squared.
		float LengthSqr(void) const;

		// return true if this vector is (0,0,0) within tolerance
		bool IsZero(float tolerance = 0.01f) const;

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

		// assignment
		Vector2D& operator=(const Vector2D& vOther);

		// 2d
		float Length2D(void) const;
		float Length2DSqr(void) const;

		/// Get the component of this vector parallel to some other given vector
		Vector2D ProjectOnto(const Vector2D& onto);

		// copy constructors
		// Vector2D(const Vector2D &vOther);

		// arithmetic operations
		Vector2D operator-(void) const;

		Vector2D operator+(const Vector2D& v) const;
		Vector2D operator-(const Vector2D& v) const;
		Vector2D operator*(const Vector2D& v) const;
		Vector2D operator/(const Vector2D& v) const;
		Vector2D operator*(float fl) const;
		Vector2D operator/(float fl) const;

		// Cross product between two vectors.
		Vector2D Cross(const Vector2D& vOther) const;

		// Returns a vector with the min or max in X, Y, and Z.
		Vector2D Min(const Vector2D& vOther) const;
		Vector2D Max(const Vector2D& vOther) const;
	};

	class Vector4D
	{
	public:
		// Members
		float x, y, z, w;

		// Construction/destruction:
		Vector4D( );
		Vector4D(float X, float Y, float Z, float W);
		Vector4D(float* clr);

		// Initialization
		void Init(float ix = 0.0f, float iy = 0.0f, float iz = 0.0f, float iw = 0.0f);
		// TODO (Ilya): Should there be an init that takes a single float for consistency?

		// Got any nasty NAN's?
		bool IsValid( ) const;
		void Invalidate( );

		// array access...
		float  operator[](int i) const;
		float& operator[](int i);

		// Base address...
		float* Base( );
		const float* Base( ) const;

		// Initialization methods
		[[deprecated]]
		void Random(float minVal, float maxVal);
		void Zero( ); ///< zero out a vector

		// equality
		bool operator==(const Vector4D& v) const;
		bool operator!=(const Vector4D& v) const;

		// arithmetic operations
		Vector4D& operator+=(const Vector4D& v);
		Vector4D& operator-=(const Vector4D& v);
		Vector4D& operator*=(float fl);
		Vector4D& operator*=(const Vector4D& v);
		Vector4D& operator/=(const Vector4D& v);

		// this ought to be an opcode.
		Vector4D& operator+=(float fl);

		// this ought to be an opcode.
		Vector4D& operator/=(float fl);

		Vector4D& operator-=(float fl);

		// negate the vector components
		void Negate( );

		// Get the vector's magnitude.
		float Length( ) const;

		// Get the vector's magnitude squared.
		float LengthSqr( ) const;

		// return true if this vector is (0,0,0) within tolerance
		bool IsZero(float tolerance = 0.01f) const;

		float    NormalizeInPlace( );
		Vector4D Normalized( ) const;
		bool     IsLengthGreaterThan(float val) const;
		bool     IsLengthLessThan(float val) const;

		// check if a vector is within the box defined by two other vectors
		bool WithinAABox(const Vector4D& boxmin, const Vector4D& boxmax);

		// Get the distance from this vector to the other one.
		float DistTo(const Vector4D& vOther) const;

		// Get the distance from this vector to the other one squared.
		// NJS: note, VC wasn't inlining it correctly in several deeply nested inlines due to being an 'out of line' .  
		// may be able to tidy this up after switching to VC7
		float DistToSqr(const Vector4D& vOther) const;

		// Copy
		void CopyToArray(float* rgfl) const;

		// Multiply, add, and assign to this (ie: *this = a + b * scalar). This
		// is about 12% faster than the actual vector equation (because it's done per-component
		// rather than per-vector).
		void MulAdd(const Vector4D& a, const Vector4D& b, float scalar);

		// Dot product.
		float Dot(const Vector4D& vOther) const;

		// assignment
		Vector4D& operator=(const Vector4D& vOther);

		// 2d
		float Length2D( ) const;
		float Length2DSqr( ) const;

		/// Get the component of this vector parallel to some other given vector
		Vector4D ProjectOnto(const Vector4D& onto);

		// copy constructors
		// Vector4D(const Vector4D &vOther);

		// arithmetic operations
		Vector4D operator-( ) const;

		Vector4D operator+(const Vector4D& v) const;
		Vector4D operator-(const Vector4D& v) const;
		Vector4D operator*(const Vector4D& v) const;
		Vector4D operator/(const Vector4D& v) const;
		Vector4D operator*(float fl) const;
		Vector4D operator/(float fl) const;

		// Returns a vector with the min or max in X, Y, and Z.
		Vector4D Min(const Vector4D& vOther) const;
		Vector4D Max(const Vector4D& vOther) const;
	};

}
