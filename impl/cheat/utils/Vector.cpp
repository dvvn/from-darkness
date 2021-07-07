#include "Vector.hpp"

using namespace cheat::utl;

Vector::Vector( )
{
	Invalidate( );
}

Vector::Vector(float X, float Y, float Z)
{
	x = X;
	y = Y;
	z = Z;
}

Vector::Vector(const float* clr)
{
	x = clr[0];
	y = clr[1];
	z = clr[2];
}

void Vector::Init(float ix, float iy, float iz)
{
	x = ix;
	y = iy;
	z = iz;
}

bool Vector::IsValid( ) const
{
	return std::isfinite(x) && std::isfinite(y) && std::isfinite(z);
}

void Vector::Invalidate( )
{
	x = y = z = std::numeric_limits<float>::infinity( );
}

float& Vector::operator[](int i)
{
	return reinterpret_cast<float*>(this)[i];
}

float Vector::operator[](int i) const
{
	return reinterpret_cast<const float*>(this)[i];
}

void Vector::Zero( )
{
	x = y = z = 0.0f;
}

bool Vector::operator==(const Vector& src) const
{
	return (src.x == x) && (src.y == y) && (src.z == z);
}

bool Vector::operator!=(const Vector& src) const
{
	return (src.x != x) || (src.y != y) || (src.z != z);
}

Vector& Vector::operator+=(const Vector& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}

Vector& Vector::operator-=(const Vector& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}

Vector& Vector::operator*=(float fl)
{
	x *= fl;
	y *= fl;
	z *= fl;
	return *this;
}

Vector& Vector::operator*=(const Vector& v)
{
	x *= v.x;
	y *= v.y;
	z *= v.z;
	return *this;
}

Vector& Vector::operator/=(const Vector& v)
{
	x /= v.x;
	y /= v.y;
	z /= v.z;
	return *this;
}

Vector& Vector::operator+=(float fl)
{
	x += fl;
	y += fl;
	z += fl;
	return *this;
}

Vector& Vector::operator/=(float fl)
{
	x /= fl;
	y /= fl;
	z /= fl;
	return *this;
}

Vector& Vector::operator-=(float fl)
{
	x -= fl;
	y -= fl;
	z -= fl;
	return *this;
}

void Vector::NormalizeInPlace( )
{
	*this = Normalized( );
}

Vector Vector::Normalized( ) const
{
	auto res = *this;
	if (const auto l = res.Length( ); l != 0.0f)
	{
		res /= l;
	}
	else
	{
		res.x = res.y = res.z = 0.0f;
	}
	return res;
}

float Vector::DistTo(const Vector& other) const
{
	Vector delta;

	delta.x = x - other.x;
	delta.y = y - other.y;
	delta.z = z - other.z;

	return delta.Length( );
}

float Vector::DistToSqr(const Vector& other) const
{
	Vector delta;

	delta.x = x - other.x;
	delta.y = y - other.y;
	delta.z = z - other.z;

	return delta.LengthSqr( );
}

float Vector::Dot(const Vector& other) const
{
	return (x * other.x + y * other.y + z * other.z);
}

float Vector::Length( ) const
{
	return sqrt(x * x + y * y + z * z);
}

float Vector::LengthSqr( ) const
{
	return (x * x + y * y + z * z);
}

float Vector::Length2D( ) const
{
	return sqrt(x * x + y * y);
}

Vector& Vector::operator=(const Vector& other)
{
	x = other.x;
	y = other.y;
	z = other.z;
	return *this;
}

Vector Vector::operator-( ) const
{
	return Vector(-x, -y, -z);
}

Vector Vector::operator+(const Vector& v) const
{
	return Vector(x + v.x, y + v.y, z + v.z);
}

Vector Vector::operator-(const Vector& v) const
{
	return Vector(x - v.x, y - v.y, z - v.z);
}

Vector Vector::operator*(float fl) const
{
	return Vector(x * fl, y * fl, z * fl);
}

Vector Vector::operator*(const Vector& v) const
{
	return Vector(x * v.x, y * v.y, z * v.z);
}

Vector Vector::operator/(float fl) const
{
	return Vector(x / fl, y / fl, z / fl);
}

Vector Vector::operator/(const Vector& v) const
{
	return Vector(x / v.x, y / v.y, z / v.z);
}

VectorAligned::VectorAligned(float X, float Y, float Z)
{
	Init(X, Y, Z);
}

VectorAligned::VectorAligned(const Vector& other)
{
	Init(other.x, other.y, other.z);
}

VectorAligned& VectorAligned::operator=(const Vector& other)
{
	Init(other.x, other.y, other.z);
	return *this;
}

VectorAligned& VectorAligned::operator=(const VectorAligned& other)
{
	Init(other.x, other.y, other.z);
	return *this;
}
