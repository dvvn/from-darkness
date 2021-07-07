#pragma once

namespace cheat::utl
{
	class Vector
	{
	public:
		Vector( );

		Vector(float X, float Y, float Z);

		Vector(const float* clr);

		void Init(float ix = 0.0f, float iy = 0.0f, float iz = 0.0f);

		bool IsValid( ) const;

		void Invalidate( );

		float& operator[](int i);

		float operator[](int i) const;

		void Zero( );

		bool operator==(const Vector& src) const;

		bool operator!=(const Vector& src) const;

		Vector& operator+=(const Vector& v);

		Vector& operator-=(const Vector& v);

		Vector& operator*=(float fl);

		Vector& operator*=(const Vector& v);

		Vector& operator/=(const Vector& v);

		Vector& operator+=(float fl);

		Vector& operator/=(float fl);

		Vector& operator-=(float fl);

		void NormalizeInPlace( );

		Vector Normalized( ) const;

		float DistTo(const Vector& other) const;

		float DistToSqr(const Vector& other) const;

		float Dot(const Vector& other) const;

		float Length( ) const;

		float LengthSqr(void) const;

		float Length2D( ) const;

		Vector& operator=(const Vector& other);

		Vector operator-(void) const;

		Vector operator+(const Vector& v) const;

		Vector operator-(const Vector& v) const;

		Vector operator*(float fl) const;

		Vector operator*(const Vector& v) const;

		Vector operator/(float fl) const;

		Vector operator/(const Vector& v) const;

		float x, y, z;
	};

	inline Vector operator*(float lhs, const Vector& rhs)
	{
		return rhs * lhs;
	}

	inline Vector operator/(float lhs, const Vector& rhs)
	{
		return rhs / lhs;
	}

	class __declspec(align(16)) VectorAligned: public Vector
	{
	public:
		VectorAligned( ) = default;;

		VectorAligned(float X, float Y, float Z);

	public:
		explicit VectorAligned(const Vector& other);

		VectorAligned& operator=(const Vector& other);

		VectorAligned& operator=(const VectorAligned& other);

		float w;
	};
}
