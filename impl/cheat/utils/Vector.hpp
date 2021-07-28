#pragma once

namespace cheat::utl
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

	class alignas(uint16_t) VectorAligned: public Vector
	{
	public:
		VectorAligned(const Vector& other = { });

		VectorAligned& operator=(const VectorAligned& other);

		float w;
	};
}
