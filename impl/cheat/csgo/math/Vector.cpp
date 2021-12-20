module;

#include <cmath>

module cheat.csgo.math.Vector;

using namespace cheat::csgo;

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

VectorAligned::VectorAligned(const Vector& other) : Vector(other)
{
}

VectorAligned& VectorAligned::operator=(const VectorAligned& other)
{
	// ReSharper disable once CppRedundantCastExpression
	*static_cast<Vector*>(this) = static_cast<const Vector&>(other);
	return *this;
}


//-------------------------

Vector2D::Vector2D(float X, float Y)
{
	x = X;
	y = Y;
}

Vector2D::Vector2D(float* clr)
{
	x = clr[0];
	y = clr[1];
}

Vector2D::Vector2D(const Vector2D& vOther)
{
	x = vOther.x;
	y = vOther.y;
}

//-----------------------------------------------------------------------------
// initialization
//-----------------------------------------------------------------------------

void Vector2D::Init(float ix, float iy)
{
	x = ix;
	y = iy;
}

void Vector2D::Random(float minVal, float maxVal)
{
	x = minVal + ((float)rand( ) / RAND_MAX) * (maxVal - minVal);
	y = minVal + ((float)rand( ) / RAND_MAX) * (maxVal - minVal);
}

void Vector2DClear(Vector2D& a)
{
	a.x = a.y = 0.0f;
}

//-----------------------------------------------------------------------------
// assignment
//-----------------------------------------------------------------------------

Vector2D& Vector2D::operator=(const Vector2D& vOther)
{
	x = vOther.x;
	y = vOther.y;
	return *this;
}

//-----------------------------------------------------------------------------
// Array access
//-----------------------------------------------------------------------------

float& Vector2D::operator[](int i)
{
	return ((float*)this)[i];
}

float Vector2D::operator[](int i) const
{
	return ((float*)this)[i];
}

//-----------------------------------------------------------------------------
// Base address...
//-----------------------------------------------------------------------------

float* Vector2D::Base( )
{
	return (float*)this;
}

float const* Vector2D::Base( ) const
{
	return (float const*)this;
}

//-----------------------------------------------------------------------------
// IsValid?
//-----------------------------------------------------------------------------

bool Vector2D::IsValid( ) const
{
	return !isinf(x) && !isinf(y);
}

//-----------------------------------------------------------------------------
// comparison
//-----------------------------------------------------------------------------

bool Vector2D::operator==(const Vector2D& src) const
{
	return (src.x == x) && (src.y == y);
}

bool Vector2D::operator!=(const Vector2D& src) const
{
	return (src.x != x) || (src.y != y);
}

Vector2D& Vector2D::operator+=(const Vector2D& v)
{
	x += v.x;
	y += v.y;
	return *this;
}

Vector2D& Vector2D::operator-=(const Vector2D& v)
{
	x -= v.x;
	y -= v.y;
	return *this;
}

Vector2D& Vector2D::operator*=(float fl)
{
	x *= fl;
	y *= fl;
	return *this;
}

Vector2D& Vector2D::operator*=(const Vector2D& v)
{
	x *= v.x;
	y *= v.y;
	return *this;
}

Vector2D& Vector2D::operator/=(const Vector2D& v)
{
	x /= v.x;
	y /= v.y;
	return *this;
}

Vector2D& Vector2D::operator+=(float fl)
{
	x += fl;
	y += fl;
	return *this;
}

Vector2D& Vector2D::operator/=(float fl)
{
	x /= fl;
	y /= fl;
	return *this;
}

Vector2D& Vector2D::operator-=(float fl)
{
	x -= fl;
	y -= fl;
	return *this;
}

float Vector2D::LengthSqr( ) const
{
	return (x * x + y * y);
}

bool Vector2D::IsZero(float tolerance) const
{
	return (x > -tolerance && x < tolerance &&
			y > -tolerance && y < tolerance);
}

float Vector2D::DistToSqr(const Vector2D& vOther) const
{
	Vector2D delta;

	delta.x = x - vOther.x;
	delta.y = y - vOther.y;

	return delta.LengthSqr( );
}

//-----------------------------------------------------------------------------
// Copy
//-----------------------------------------------------------------------------

void Vector2DCopy(const Vector2D& src, Vector2D& dst)
{
	dst.x = src.x;
	dst.y = src.y;
}

void Vector2D::CopyToArray(float* rgfl) const
{
	rgfl[0] = x;
	rgfl[1] = y;
}

//-----------------------------------------------------------------------------
// standard Math operations
//-----------------------------------------------------------------------------

void Vector2D::Negate( )
{
	x = -x;
	y = -y;
}

void Vector2DAdd(const Vector2D& a, const Vector2D& b, Vector2D& c)
{
	c.x = a.x + b.x;
	c.y = a.y + b.y;
}

void Vector2DSubtract(const Vector2D& a, const Vector2D& b, Vector2D& c)
{
	c.x = a.x - b.x;
	c.y = a.y - b.y;
}

void Vector2DMultiply(const Vector2D& a, float b, Vector2D& c)
{
	c.x = a.x * b;
	c.y = a.y * b;
}

void Vector2DMultiply(const Vector2D& a, const Vector2D& b, Vector2D& c)
{
	c.x = a.x * b.x;
	c.y = a.y * b.y;
}

void Vector2DDivide(const Vector2D& a, float b, Vector2D& c)
{
	float oob = 1.0f / b;
	c.x       = a.x * oob;
	c.y       = a.y * oob;
}

void Vector2DDivide(const Vector2D& a, const Vector2D& b, Vector2D& c)
{
	c.x = a.x / b.x;
	c.y = a.y / b.y;
}

void Vector2DMA(const Vector2D& start, float s, const Vector2D& dir, Vector2D& result)
{
	result.x = start.x + s * dir.x;
	result.y = start.y + s * dir.y;
}

// FIXME: Remove
// For backwards compatability
void Vector2D::MulAdd(const Vector2D& a, const Vector2D& b, float scalar)
{
	x = a.x + b.x * scalar;
	y = a.y + b.y * scalar;
}

void Vector2DLerp(const Vector2D& src1, const Vector2D& src2, float t, Vector2D& dest)
{
	dest[0] = src1[0] + (src2[0] - src1[0]) * t;
	dest[1] = src1[1] + (src2[1] - src1[1]) * t;
}

//-----------------------------------------------------------------------------
// dot, cross
//-----------------------------------------------------------------------------
float DotProduct2D(const Vector2D& a, const Vector2D& b)
{
	return (a.x * b.x + a.y * b.y);
}

// for backwards compatability
float Vector2D::Dot(const Vector2D& vOther) const
{
	return DotProduct2D(*this, vOther);
}

float Vector2DNormalize(Vector2D& v)
{
	float l = v.Length( );
	if (l != 0.0f)
	{
		v /= l;
	}
	else
	{
		v.x = v.y = 0.0f;
	}
	return l;
}

//-----------------------------------------------------------------------------
// length
//-----------------------------------------------------------------------------
float Vector2DLength(const Vector2D& v)
{
	return (float)sqrt(v.x * v.x + v.y * v.y);
}

float Vector2D::NormalizeInPlace( )
{
	return Vector2DNormalize(*this);
}

bool Vector2D::IsLengthGreaterThan(float val) const
{
	return LengthSqr( ) > val * val;
}

bool Vector2D::IsLengthLessThan(float val) const
{
	return LengthSqr( ) < val * val;
}

float Vector2D::Length(void) const
{
	return Vector2DLength(*this);
}

void Vector2DMin(const Vector2D& a, const Vector2D& b, Vector2D& result)
{
	result.x = (a.x < b.x) ? a.x : b.x;
	result.y = (a.y < b.y) ? a.y : b.y;
}

void Vector2DMax(const Vector2D& a, const Vector2D& b, Vector2D& result)
{
	result.x = (a.x > b.x) ? a.x : b.x;
	result.y = (a.y > b.y) ? a.y : b.y;
}

//-----------------------------------------------------------------------------
// Computes the closest point to vecTarget no farther than flMaxDist from vecStart
//-----------------------------------------------------------------------------
void ComputeClosestPoint2D(const Vector2D& vecStart, float flMaxDist, const Vector2D& vecTarget, Vector2D* pResult)
{
	Vector2D vecDelta;
	Vector2DSubtract(vecTarget, vecStart, vecDelta);
	float flDistSqr = vecDelta.LengthSqr( );
	if (flDistSqr <= flMaxDist * flMaxDist)
	{
		*pResult = vecTarget;
	}
	else
	{
		vecDelta /= sqrt(flDistSqr);
		Vector2DMA(vecStart, flMaxDist, vecDelta, *pResult);
	}
}

//-----------------------------------------------------------------------------
// Returns a Vector2D with the min or max in X, Y, and Z.
//-----------------------------------------------------------------------------

Vector2D Vector2D::Min(const Vector2D& vOther) const
{
	return Vector2D(x < vOther.x ? x : vOther.x, y < vOther.y ? y : vOther.y);
}

Vector2D Vector2D::Max(const Vector2D& vOther) const
{
	return Vector2D(x > vOther.x ? x : vOther.x, y > vOther.y ? y : vOther.y);
}

//-----------------------------------------------------------------------------
// arithmetic operations
//-----------------------------------------------------------------------------

Vector2D Vector2D::operator-(void) const
{
	return Vector2D(-x, -y);
}

Vector2D Vector2D::operator+(const Vector2D& v) const
{
	Vector2D res;
	Vector2DAdd(*this, v, res);
	return res;
}

Vector2D Vector2D::operator-(const Vector2D& v) const
{
	Vector2D res;
	Vector2DSubtract(*this, v, res);
	return res;
}

Vector2D Vector2D::operator*(float fl) const
{
	Vector2D res;
	Vector2DMultiply(*this, fl, res);
	return res;
}

Vector2D Vector2D::operator*(const Vector2D& v) const
{
	Vector2D res;
	Vector2DMultiply(*this, v, res);
	return res;
}

Vector2D Vector2D::operator/(float fl) const
{
	Vector2D res;
	Vector2DDivide(*this, fl, res);
	return res;
}

Vector2D Vector2D::operator/(const Vector2D& v) const
{
	Vector2D res;
	Vector2DDivide(*this, v, res);
	return res;
}

Vector2D operator*(float fl, const Vector2D& v)
{
	return v * fl;
}

//-----------------------

void VectorCopy(const Vector4D& src, Vector4D& dst)
{
	dst.x = src.x;
	dst.y = src.y;
	dst.z = src.z;
	dst.w = src.w;
}

void VectorLerp(const Vector4D& src1, const Vector4D& src2, float t, Vector4D& dest)
{
	dest.x = src1.x + (src2.x - src1.x) * t;
	dest.y = src1.y + (src2.y - src1.y) * t;
	dest.z = src1.z + (src2.z - src1.z) * t;
	dest.w = src1.w + (src2.w - src1.w) * t;
}

float VectorLength(const Vector4D& v)
{
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}

float NormalizeVector(Vector4D& v)
{
	auto l = v.Length( );
	if (l != 0.0f)
	{
		v /= l;
	}
	else
	{
		v.x = v.y = v.z = v.w = 0.0f;
	}
	return l;
}

Vector4D::Vector4D( )
{
	Invalidate( );
}

Vector4D::Vector4D(float X, float Y, float Z, float W)
{
	x = X;
	y = Y;
	z = Z;
	w = W;
}

Vector4D::Vector4D(float* clr)
{
	x = clr[0];
	y = clr[1];
	z = clr[2];
	w = clr[3];
}

//-----------------------------------------------------------------------------
// initialization
//-----------------------------------------------------------------------------

void Vector4D::Init(float ix, float iy, float iz, float iw)
{
	x = ix;
	y = iy;
	z = iz;
	w = iw;
}

void Vector4D::Random(float minVal, float maxVal)
{
	x = minVal + (float)rand( ) / RAND_MAX * (maxVal - minVal);
	y = minVal + (float)rand( ) / RAND_MAX * (maxVal - minVal);
	z = minVal + (float)rand( ) / RAND_MAX * (maxVal - minVal);
	w = minVal + (float)rand( ) / RAND_MAX * (maxVal - minVal);
}

// This should really be a single opcode on the PowerPC (move r0 onto the vec reg)
void Vector4D::Zero( )
{
	x = y = z = w = 0.0f;
}

//-----------------------------------------------------------------------------
// assignment
//-----------------------------------------------------------------------------

Vector4D& Vector4D::operator=(const Vector4D& vOther)
{
	x = vOther.x;
	y = vOther.y;
	z = vOther.z;
	w = vOther.w;
	return *this;
}

//-----------------------------------------------------------------------------
// Array access
//-----------------------------------------------------------------------------
float& Vector4D::operator[](int i)
{
	return ((float*)this)[i];
}

float Vector4D::operator[](int i) const
{
	return ((float*)this)[i];
}

//-----------------------------------------------------------------------------
// Base address...
//-----------------------------------------------------------------------------
float* Vector4D::Base( )
{
	return (float*)this;
}

float const* Vector4D::Base( ) const
{
	return (float const*)this;
}

//-----------------------------------------------------------------------------
// IsValid?
//-----------------------------------------------------------------------------

bool Vector4D::IsValid( ) const
{
	return !isinf(x) && !isinf(y) && !isinf(z) && !isinf(w);
}

//-----------------------------------------------------------------------------
// Invalidate
//-----------------------------------------------------------------------------

void Vector4D::Invalidate( )
{
	//#ifdef _DEBUG
	//#ifdef VECTOR_PARANOIA
	x = y = z = w = std::numeric_limits<float>::infinity( );
	//#endif
	//#endif
}

//-----------------------------------------------------------------------------
// comparison
//-----------------------------------------------------------------------------

bool Vector4D::operator==(const Vector4D& src) const
{
	return src.x == x && src.y == y && src.z == z && src.w == w;
}

bool Vector4D::operator!=(const Vector4D& src) const
{
	return src.x != x || src.y != y || src.z != z || src.w != w;
}

Vector4D& Vector4D::operator+=(const Vector4D& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	w += v.w;
	return *this;
}

Vector4D& Vector4D::operator-=(const Vector4D& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	w -= v.w;
	return *this;
}

Vector4D& Vector4D::operator*=(float fl)
{
	x *= fl;
	y *= fl;
	z *= fl;
	w *= fl;
	return *this;
}

Vector4D& Vector4D::operator*=(const Vector4D& v)
{
	x *= v.x;
	y *= v.y;
	z *= v.z;
	w *= v.w;
	return *this;
}

Vector4D& Vector4D::operator/=(const Vector4D& v)
{
	x /= v.x;
	y /= v.y;
	z /= v.z;
	w /= v.w;
	return *this;
}

Vector4D& Vector4D::operator+=(float fl)
{
	x += fl;
	y += fl;
	z += fl;
	w += fl;
	return *this;
}

Vector4D& Vector4D::operator/=(float fl)
{
	x /= fl;
	y /= fl;
	z /= fl;
	w /= fl;
	return *this;
}

Vector4D& Vector4D::operator-=(float fl)
{
	x -= fl;
	y -= fl;
	z -= fl;
	w -= fl;
	return *this;
}

float Vector4D::LengthSqr( ) const
{
	return x * x + y * y + z * z;
}

//-----------------------------------------------------------------------------
// Copy
//-----------------------------------------------------------------------------
void Vector4D::CopyToArray(float* rgfl) const
{
	rgfl[0] = x, rgfl[1] = y, rgfl[2] = z;
	rgfl[3] = w;
}

//-----------------------------------------------------------------------------
// standard Math operations
//-----------------------------------------------------------------------------
// #pragma message("TODO: these should be SSE")

void Vector4D::Negate( )
{
	x = -x;
	y = -y;
	z = -z;
	w = -w;
}

// Get the component of this vector parallel to some other given vector
Vector4D Vector4D::ProjectOnto(const Vector4D& onto)
{
	return onto * (this->Dot(onto) / onto.LengthSqr( ));
}

// FIXME: Remove
// For backwards compatability
void Vector4D::MulAdd(const Vector4D& a, const Vector4D& b, float scalar)
{
	x = a.x + b.x * scalar;
	y = a.y + b.y * scalar;
	z = a.z + b.z * scalar;
	w = a.w + b.w * scalar;
}

Vector4D VectorLerp(const Vector4D& src1, const Vector4D& src2, float t)
{
	Vector4D result;
	VectorLerp(src1, src2, t, result);
	return result;
}

float Vector4D::Dot(const Vector4D& b) const
{
	return x * b.x + y * b.y + z * b.z + w * b.w;
}

void VectorClear(Vector4D& a)
{
	a.x = a.y = a.z = a.w = 0.0f;
}

float Vector4D::Length( ) const
{
	return sqrt(x * x + y * y + z * z + w * w);
}

bool Vector4D::IsZero(float tolerance) const
{
	return x > -tolerance && x < tolerance &&
			y > -tolerance && y < tolerance &&
			z > -tolerance && z < tolerance &&
			w > -tolerance && w < tolerance;
}

// check a point against a box
bool Vector4D::WithinAABox(Vector4D const& boxmin, Vector4D const& boxmax)
{
	return x >= boxmin.x && x <= boxmax.x &&
			y >= boxmin.y && y <= boxmax.y &&
			z >= boxmin.z && z <= boxmax.z &&
			w >= boxmin.w && w <= boxmax.w;
}

//-----------------------------------------------------------------------------
// Get the distance from this vector to the other one 
//-----------------------------------------------------------------------------
float Vector4D::DistTo(const Vector4D& vOther) const
{
	Vector4D delta;
	delta = *this - vOther;
	return delta.Length( );
}

float Vector4D::DistToSqr(const Vector4D& vOther) const
{
	Vector4D delta;

	delta.x = x - vOther.x;
	delta.y = y - vOther.y;
	delta.z = z - vOther.z;
	delta.w = w - vOther.w;

	return delta.LengthSqr( );
}

//-----------------------------------------------------------------------------
// Returns a vector with the min or max in X, Y, and Z.
//-----------------------------------------------------------------------------
Vector4D Vector4D::Min(const Vector4D& vOther) const
{
	return Vector4D(x < vOther.x ? x : vOther.x,
					y < vOther.y ? y : vOther.y,
					z < vOther.z ? z : vOther.z,
					w < vOther.w ? w : vOther.w);
}

Vector4D Vector4D::Max(const Vector4D& vOther) const
{
	return Vector4D(x > vOther.x ? x : vOther.x,
					y > vOther.y ? y : vOther.y,
					z > vOther.z ? z : vOther.z,
					w > vOther.w ? w : vOther.w);
}

//-----------------------------------------------------------------------------
// arithmetic operations
//-----------------------------------------------------------------------------

Vector4D Vector4D::operator-( ) const
{
	return Vector4D(-x, -y, -z, -w);
}

Vector4D Vector4D::operator+(const Vector4D& v) const
{
	return Vector4D(x + v.x, y + v.y, z + v.z, w + v.w);
}

Vector4D Vector4D::operator-(const Vector4D& v) const
{
	return Vector4D(x - v.x, y - v.y, z - v.z, w - v.w);
}

Vector4D Vector4D::operator*(float fl) const
{
	return Vector4D(x * fl, y * fl, z * fl, w * fl);
}

Vector4D Vector4D::operator*(const Vector4D& v) const
{
	return Vector4D(x * v.x, y * v.y, z * v.z, w * v.w);
}

Vector4D Vector4D::operator/(float fl) const
{
	return Vector4D(x / fl, y / fl, z / fl, w / fl);
}

Vector4D Vector4D::operator/(const Vector4D& v) const
{
	return Vector4D(x / v.x, y / v.y, z / v.z, w / v.w);
}

Vector4D operator*(float fl, const Vector4D& v)
{
	return v * fl;
}