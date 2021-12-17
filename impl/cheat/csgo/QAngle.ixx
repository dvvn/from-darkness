module;

#include <cmath>

export module cheat.csgo.math:QAngle;

export namespace cheat::csgo
{
	class QAngle
	{
	public:
		QAngle()
		{
			Init();
		}

		QAngle(float X, float Y, float Z)
		{
			Init(X, Y, Z);
		}

		QAngle(const float* clr)
		{
			Init(clr[0], clr[1], clr[2]);
		}

		void Init(float ix = 0, float iy = 0, float iz = 0)
		{
			pitch = ix;
			yaw = iy;
			roll = iz;
		}

		float operator[](int i) const
		{
			return ((float*)this)[i];
		}

		float& operator[](int i)
		{
			return ((float*)this)[i];
		}

		QAngle& operator+=(const QAngle& v)
		{
			pitch += v.pitch;
			yaw += v.yaw;
			roll += v.roll;
			return *this;
		}

		QAngle& operator-=(const QAngle& v)
		{
			pitch -= v.pitch;
			yaw -= v.yaw;
			roll -= v.roll;
			return *this;
		}

		QAngle& operator*=(float fl)
		{
			pitch *= fl;
			yaw *= fl;
			roll *= fl;
			return *this;
		}

		QAngle& operator*=(const QAngle& v)
		{
			pitch *= v.pitch;
			yaw *= v.yaw;
			roll *= v.roll;
			return *this;
		}

		QAngle& operator/=(const QAngle& v)
		{
			pitch /= v.pitch;
			yaw /= v.yaw;
			roll /= v.roll;
			return *this;
		}

		QAngle& operator+=(float fl)
		{
			pitch += fl;
			yaw += fl;
			roll += fl;
			return *this;
		}

		QAngle& operator/=(float fl)
		{
			pitch /= fl;
			yaw /= fl;
			roll /= fl;
			return *this;
		}

		QAngle& operator-=(float fl)
		{
			pitch -= fl;
			yaw -= fl;
			roll -= fl;
			return *this;
		}

		QAngle& operator=(const QAngle& vOther)
		{
			pitch = vOther.pitch;
			yaw = vOther.yaw;
			roll = vOther.roll;
			return *this;
		}

		QAngle operator-() const
		{
			return QAngle(-pitch, -yaw, -roll);
		}

		QAngle operator+(const QAngle& v) const
		{
			return QAngle(pitch + v.pitch, yaw + v.yaw, roll + v.roll);
		}

		QAngle operator-(const QAngle& v) const
		{
			return QAngle(pitch - v.pitch, yaw - v.yaw, roll - v.roll);
		}

		QAngle operator*(float fl) const
		{
			return QAngle(pitch * fl, yaw * fl, roll * fl);
		}

		QAngle operator*(const QAngle& v) const
		{
			return QAngle(pitch * v.pitch, yaw * v.yaw, roll * v.roll);
		}

		QAngle operator/(float fl) const
		{
			return QAngle(pitch / fl, yaw / fl, roll / fl);
		}

		QAngle operator/(const QAngle& v) const
		{
			return QAngle(pitch / v.pitch, yaw / v.yaw, roll / v.roll);
		}

		float Length() const
		{
			return std::sqrt(pitch * pitch + yaw * yaw + roll * roll);
		}

		float LengthSqr() const
		{
			return (pitch * pitch + yaw * yaw + roll * roll);
		}

		bool IsZero(float tolerance) const
		{
			return (pitch > -tolerance && pitch < tolerance&& yaw > -tolerance && yaw < tolerance&& roll > -tolerance && roll < tolerance);
		}

		float Normalize() const
		{
			QAngle res = *this;
			float  l = res.Length();
			if (l != 0.0f)
			{
				res /= l;
			}
			else
			{
				res[0] = res[1] = res[2] = 0.0f;
			}
			return l;
		}

		float pitch;
		float yaw;
		float roll;
	};

	inline QAngle operator*(float lhs, const QAngle& rhs)
	{
		return rhs * lhs;
	}

	inline QAngle operator/(float lhs, const QAngle& rhs)
	{
		return rhs / lhs;
	}
}
