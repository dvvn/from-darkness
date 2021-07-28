#include "Vector.hpp"

using namespace cheat::utl;

Vector::Vector(float val)
{
	x = y = z = val;
}

Vector::Vector(float X, float Y, float Z)
{
	x = X;
	y = Y;
	z = Z;
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
		res = 0.0f;
	}
	return res;
}

float Vector::DistTo(const Vector& other) const
{
	const auto delta = *this - other;
	return delta.Length( );
}

float Vector::DistToSqr(const Vector& other) const
{
	const auto delta = *this - other;
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

Vector Vector::operator*(const Vector& v) const
{
	return Vector(x * v.x, y * v.y, z * v.z);
}

Vector Vector::operator/(const Vector& v) const
{
	return Vector(x / v.x, y / v.y, z / v.z);
}

bool Vector::operator==(const Vector& v) const
{
	return x == v.x && y == v.y && z == v.z;
}

bool Vector::operator!=(const Vector& v) const
{
	return !(*this == v);
}

VectorAligned::VectorAligned(const Vector& other): Vector(other)
{
}

VectorAligned& VectorAligned::operator=(const VectorAligned& other)
{
	// ReSharper disable once CppRedundantCastExpression
	*static_cast<Vector*>(this) = static_cast<const Vector&>(other);
	return *this;
}
