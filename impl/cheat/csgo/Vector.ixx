module;

#include <cstdint>
#include <limits>
#include <cmath>

export module cheat.csgo.math:Vector;

export namespace cheat::csgo
{
	class Vector
	{
	public:
		Vector(float val = std::numeric_limits<float>::infinity())
		{
			x = y = z = val;
		}

		Vector(float X, float Y, float Z)
		{
			x = X;
			y = Y;
			z = Z;
		}

		bool IsValid() const
		{
			return std::isfinite(x) && std::isfinite(y) && std::isfinite(z);
		}

		void Invalidate()
		{
			x = y = z = std::numeric_limits<float>::infinity();
		}

		float& operator[](int i)
		{
			return reinterpret_cast<float*>(this)[i];
		}

		float operator[](int i) const
		{
			return reinterpret_cast<const float*>(this)[i];
		}

		Vector& operator+=(const Vector& v)
		{
			x += v.x;
			y += v.y;
			z += v.z;
			return *this;
		}

		Vector& operator-=(const Vector& v)
		{
			x -= v.x;
			y -= v.y;
			z -= v.z;
			return *this;
		}

		Vector& operator*=(const Vector& v)
		{
			x *= v.x;
			y *= v.y;
			z *= v.z;
			return *this;
		}

		Vector& operator/=(const Vector& v)
		{
			x /= v.x;
			y /= v.y;
			z /= v.z;
			return *this;
		}

		void NormalizeInPlace()
		{
			*this = Normalized();
		}

		Vector Normalized() const
		{
			auto res = *this;
			if (const auto l = res.Length(); l != 0.0f)
			{
				res /= l;
			}
			else
			{
				res = 0.0f;
			}
			return res;
		}

		float DistTo(const Vector& other) const
		{
			const auto delta = *this - other;
			return delta.Length();
		}

		float DistToSqr(const Vector& other) const
		{
			const auto delta = *this - other;
			return delta.LengthSqr();
		}

		float Dot(const Vector& other) const
		{
			return (x * other.x + y * other.y + z * other.z);
		}

		float Length() const
		{
			return sqrt(x * x + y * y + z * z);
		}

		float LengthSqr() const
		{
			return (x * x + y * y + z * z);
		}

		float Length2D() const
		{
			return sqrt(x * x + y * y);
		}

		Vector operator-() const
		{
			return Vector(-x, -y, -z);
		}

		Vector operator+(const Vector& v) const
		{
			return Vector(x + v.x, y + v.y, z + v.z);
		}

		Vector operator-(const Vector& v) const
		{
			return Vector(x - v.x, y - v.y, z - v.z);
		}

		Vector operator*(const Vector& v) const
		{
			return Vector(x * v.x, y * v.y, z * v.z);
		}

		Vector operator/(const Vector& v) const
		{
			return Vector(x / v.x, y / v.y, z / v.z);
		}

		bool operator==(const Vector& v) const
		{
			return x == v.x && y == v.y && z == v.z;
		}

		bool operator!=(const Vector& v) const
		{
			return !(*this == v);
		}



		float x, y, z;
	};

	class alignas(uint16_t) VectorAligned : public Vector
	{
	public:
		VectorAligned(const Vector& other = {}) : Vector(other)
		{
		}

		VectorAligned& operator=(const VectorAligned& other)
		{
			*static_cast<Vector*>(this) = static_cast<const Vector&>(other);
			return *this;
		}

		float w;
	};

	//----------------

	class Vector2D
	{
	public:


		Vector2D(float X = 0, float Y = 0)
		{
			x = X;
			y = Y;
		}

		Vector2D(float* clr)
		{
			x = clr[0];
			y = clr[1];
		}

		Vector2D(const Vector2D& vOther)
		{
			x = vOther.x;
			y = vOther.y;
		}

		//-----------------------------------------------------------------------------
		// initialization
		//-----------------------------------------------------------------------------

		void Init(float ix = 0.0f, float iy = 0.0f)
		{
			x = ix;
			y = iy;
		}

		Vector2D& operator=(const Vector2D& vOther)
		{
			x = vOther.x;
			y = vOther.y;
			return *this;
		}

		void Random(float minVal, float maxVal)
		{
			x = minVal + ((float)rand() / RAND_MAX) * (maxVal - minVal);
			y = minVal + ((float)rand() / RAND_MAX) * (maxVal - minVal);
		}

		float& operator[](int i)
		{
			return ((float*)this)[i];
		}

		float operator[](int i) const
		{
			return ((float*)this)[i];
		}

		//-----------------------------------------------------------------------------
		// Base address...
		//-----------------------------------------------------------------------------

		float* Base()
		{
			return (float*)this;
		}

		float const* Base() const
		{
			return (float const*)this;
		}

		//-----------------------------------------------------------------------------
		// IsValid?
		//-----------------------------------------------------------------------------

		bool IsValid() const
		{
			return !isinf(x) && !isinf(y);
		}

		//-----------------------------------------------------------------------------
		// comparison
		//-----------------------------------------------------------------------------

		bool operator==(const Vector2D& src) const
		{
			return (src.x == x) && (src.y == y);
		}



		bool operator!=(const Vector2D& src) const
		{
			return (src.x != x) || (src.y != y);
		}

		Vector2D& operator+=(const Vector2D& v)
		{
			x += v.x;
			y += v.y;
			return *this;
		}

		Vector2D& operator-=(const Vector2D& v)
		{
			x -= v.x;
			y -= v.y;
			return *this;
		}

		Vector2D& operator*=(float fl)
		{
			x *= fl;
			y *= fl;
			return *this;
		}

		Vector2D& operator*=(const Vector2D& v)
		{
			x *= v.x;
			y *= v.y;
			return *this;
		}

		Vector2D& operator/=(const Vector2D& v)
		{
			x /= v.x;
			y /= v.y;
			return *this;
		}

		Vector2D& operator+=(float fl)
		{
			x += fl;
			y += fl;
			return *this;
		}

		Vector2D& operator/=(float fl)
		{
			x /= fl;
			y /= fl;
			return *this;
		}

		Vector2D& operator-=(float fl)
		{
			x -= fl;
			y -= fl;
			return *this;
		}

		float LengthSqr() const
		{
			return (x * x + y * y);
		}

		bool IsZero(float tolerance = 0.01f) const
		{
			return (x > -tolerance && x < tolerance&&
				y > -tolerance && y < tolerance);
		}

		float DistToSqr(const Vector2D& vOther) const
		{
			Vector2D delta;

			delta.x = x - vOther.x;
			delta.y = y - vOther.y;

			return delta.LengthSqr();
		}

		void CopyToArray(float* rgfl) const
		{
			rgfl[0] = x;
			rgfl[1] = y;
		}

		//-----------------------------------------------------------------------------
		// standard Math operations
		//-----------------------------------------------------------------------------

		void Negate()
		{
			x = -x;
			y = -y;
		}

		// FIXME: Remove
// For backwards compatability
		void MulAdd(const Vector2D& a, const Vector2D& b, float scalar)
		{
			x = a.x + b.x * scalar;
			y = a.y + b.y * scalar;
		}

		// for backwards compatability
		float Dot(const Vector2D& vOther) const
		{
			auto& a = *this;
			auto& b = vOther;
			return (a.x * b.x + a.y * b.y);
		}


		float NormalizeInPlace()
		{
			auto& v = *this;
			float l = v.Length();
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

		bool IsLengthGreaterThan(float val) const
		{
			return LengthSqr() > val * val;
		}

		bool IsLengthLessThan(float val) const
		{
			return LengthSqr() < val * val;
		}

		float Length(void) const
		{
			auto& v = *this;
			return sqrt(v.x * v.x + v.y * v.y);
		}

		Vector2D Min(const Vector2D& vOther) const
		{
			return Vector2D(x < vOther.x ? x : vOther.x, y < vOther.y ? y : vOther.y);
		}

		Vector2D Max(const Vector2D& vOther) const
		{
			return Vector2D(x > vOther.x ? x : vOther.x, y > vOther.y ? y : vOther.y);
		}

		//-----------------------------------------------------------------------------
		// arithmetic operations
		//-----------------------------------------------------------------------------

		Vector2D operator-(void) const
		{
			return Vector2D(-x, -y);
		}

		Vector2D operator+(const Vector2D& v) const
		{
			auto& a = *this;
			auto& b = v;
			Vector2D c;

			c.x = a.x + b.x;
			c.y = a.y + b.y;

			return c;
		}

		Vector2D operator-(const Vector2D& v) const
		{
			return operator+(-v);

		}

		Vector2D operator*(const Vector2D& v) const
		{
			auto& a = *this;
			auto& b = v;
			Vector2D c;

			c.x = a.x * b.x;
			c.y = a.y * b.y;

			return c;
		}


		Vector2D operator/(const Vector2D& v) const
		{
			auto& a = *this;
			auto& b = v;
			Vector2D c;

			c.x = a.x / b.x;
			c.y = a.y / b.y;

			return c;
		}



		// Members
		float x, y;


	};


	class Vector4D
	{
	public:

		Vector4D()
		{
			Invalidate();
		}

		Vector4D(float X, float Y, float Z, float W)
		{
			x = X;
			y = Y;
			z = Z;
			w = W;
		}

		Vector4D(float* clr)
		{
			x = clr[0];
			y = clr[1];
			z = clr[2];
			w = clr[3];
		}


		void Init(float ix, float iy, float iz, float iw)
		{
			x = ix;
			y = iy;
			z = iz;
			w = iw;
		}

		void Random(float minVal, float maxVal)
		{
			x = minVal + (float)rand() / RAND_MAX * (maxVal - minVal);
			y = minVal + (float)rand() / RAND_MAX * (maxVal - minVal);
			z = minVal + (float)rand() / RAND_MAX * (maxVal - minVal);
			w = minVal + (float)rand() / RAND_MAX * (maxVal - minVal);
		}

		// This should really be a single opcode on the PowerPC (move r0 onto the vec reg)
		void Zero()
		{
			x = y = z = w = 0.0f;
		}

		Vector4D& operator=(const Vector4D& vOther)
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
		float& operator[](int i)
		{
			return ((float*)this)[i];
		}

		float operator[](int i) const
		{
			return ((float*)this)[i];
		}

		//-----------------------------------------------------------------------------
		// Base address...
		//-----------------------------------------------------------------------------
		float* Base()
		{
			return (float*)this;
		}

		float const* Base() const
		{
			return (float const*)this;
		}

		//-----------------------------------------------------------------------------
		// IsValid?
		//-----------------------------------------------------------------------------

		bool IsValid() const
		{
			return !isinf(x) && !isinf(y) && !isinf(z) && !isinf(w);
		}

		//-----------------------------------------------------------------------------
		// Invalidate
		//-----------------------------------------------------------------------------

		void Invalidate()
		{
			//#ifdef _DEBUG
			//#ifdef VECTOR_PARANOIA
			x = y = z = w = std::numeric_limits<float>::infinity();
			//#endif
			//#endif
		}

		//-----------------------------------------------------------------------------
		// comparison
		//-----------------------------------------------------------------------------

		bool operator==(const Vector4D& src) const
		{
			return src.x == x && src.y == y && src.z == z && src.w == w;
		}

		bool operator!=(const Vector4D& src) const
		{
			return src.x != x || src.y != y || src.z != z || src.w != w;
		}

		Vector4D& operator+=(const Vector4D& v)
		{
			x += v.x;
			y += v.y;
			z += v.z;
			w += v.w;
			return *this;
		}

		Vector4D& operator-=(const Vector4D& v)
		{
			x -= v.x;
			y -= v.y;
			z -= v.z;
			w -= v.w;
			return *this;
		}

		Vector4D& operator*=(float fl)
		{
			x *= fl;
			y *= fl;
			z *= fl;
			w *= fl;
			return *this;
		}

		Vector4D& operator*=(const Vector4D& v)
		{
			x *= v.x;
			y *= v.y;
			z *= v.z;
			w *= v.w;
			return *this;
		}

		Vector4D& operator/=(const Vector4D& v)
		{
			x /= v.x;
			y /= v.y;
			z /= v.z;
			w /= v.w;
			return *this;
		}

		Vector4D& operator+=(float fl)
		{
			x += fl;
			y += fl;
			z += fl;
			w += fl;
			return *this;
		}

		Vector4D& operator/=(float fl)
		{
			x /= fl;
			y /= fl;
			z /= fl;
			w /= fl;
			return *this;
		}

		Vector4D& operator-=(float fl)
		{
			x -= fl;
			y -= fl;
			z -= fl;
			w -= fl;
			return *this;
		}

		float LengthSqr() const
		{
			return x * x + y * y + z * z;
		}

		//-----------------------------------------------------------------------------
		// Copy
		//-----------------------------------------------------------------------------
		void CopyToArray(float* rgfl) const
		{
			rgfl[0] = x, rgfl[1] = y, rgfl[2] = z;
			rgfl[3] = w;
		}

		//-----------------------------------------------------------------------------
		// standard Math operations
		//-----------------------------------------------------------------------------
		// #pragma message("TODO: these should be SSE")

		void Negate()
		{
			x = -x;
			y = -y;
			z = -z;
			w = -w;
		}

		// Get the component of this vector parallel to some other given vector
		Vector4D ProjectOnto(const Vector4D& onto)
		{
			return onto * (this->Dot(onto) / onto.LengthSqr());
		}

		// FIXME: Remove
		// For backwards compatability
		void MulAdd(const Vector4D& a, const Vector4D& b, float scalar)
		{
			x = a.x + b.x * scalar;
			y = a.y + b.y * scalar;
			z = a.z + b.z * scalar;
			w = a.w + b.w * scalar;
		}

		float Dot(const Vector4D& b) const
		{
			return x * b.x + y * b.y + z * b.z + w * b.w;
		}

		void VectorClear(Vector4D& a)
		{
			a.x = a.y = a.z = a.w = 0.0f;
		}

		float Length() const
		{
			return std::sqrt(x * x + y * y + z * z + w * w);
		}

		bool IsZero(float tolerance) const
		{
			return x > -tolerance && x < tolerance&&
				y > -tolerance && y < tolerance&&
				z > -tolerance && z < tolerance&&
				w > -tolerance && w < tolerance;
		}

		// check a point against a box
		bool WithinAABox(Vector4D const& boxmin, Vector4D const& boxmax)
		{
			return x >= boxmin.x && x <= boxmax.x &&
				y >= boxmin.y && y <= boxmax.y &&
				z >= boxmin.z && z <= boxmax.z &&
				w >= boxmin.w && w <= boxmax.w;
		}

		//-----------------------------------------------------------------------------
		// Get the distance from this vector to the other one 
		//-----------------------------------------------------------------------------
		float DistTo(const Vector4D& vOther) const
		{
			Vector4D delta;
			delta = *this - vOther;
			return delta.Length();
		}

		float DistToSqr(const Vector4D& vOther) const
		{
			Vector4D delta;

			delta.x = x - vOther.x;
			delta.y = y - vOther.y;
			delta.z = z - vOther.z;
			delta.w = w - vOther.w;

			return delta.LengthSqr();
		}

		//-----------------------------------------------------------------------------
		// Returns a vector with the min or max in X, Y, and Z.
		//-----------------------------------------------------------------------------
		Vector4D Min(const Vector4D& vOther) const
		{
			return Vector4D(x < vOther.x ? x : vOther.x,
				y < vOther.y ? y : vOther.y,
				z < vOther.z ? z : vOther.z,
				w < vOther.w ? w : vOther.w);
		}

		Vector4D Max(const Vector4D& vOther) const
		{
			return Vector4D(x > vOther.x ? x : vOther.x,
				y > vOther.y ? y : vOther.y,
				z > vOther.z ? z : vOther.z,
				w > vOther.w ? w : vOther.w);
		}

		//-----------------------------------------------------------------------------
		// arithmetic operations
		//-----------------------------------------------------------------------------

		Vector4D operator-() const
		{
			return Vector4D(-x, -y, -z, -w);
		}

		Vector4D operator+(const Vector4D& v) const
		{
			return Vector4D(x + v.x, y + v.y, z + v.z, w + v.w);
		}

		Vector4D operator-(const Vector4D& v) const
		{
			return Vector4D(x - v.x, y - v.y, z - v.z, w - v.w);
		}

		Vector4D operator*(float fl) const
		{
			return Vector4D(x * fl, y * fl, z * fl, w * fl);
		}

		Vector4D operator*(const Vector4D& v) const
		{
			return Vector4D(x * v.x, y * v.y, z * v.z, w * v.w);
		}

		Vector4D operator/(float fl) const
		{
			return Vector4D(x / fl, y / fl, z / fl, w / fl);
		}

		Vector4D operator/(const Vector4D& v) const
		{
			return Vector4D(x / v.x, y / v.y, z / v.z, w / v.w);
		}

		// Members
		float x, y, z, w;


	};

}
