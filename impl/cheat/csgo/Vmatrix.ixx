module;

#include <cstring>
#include <limits>

export module cheat.csgo.math:Vmatrix;
export import :QAngle;
export import :Vector;

export namespace cheat::csgo
{
	class matrix3x4_t
	{
	public:
		matrix3x4_t() = default;

		matrix3x4_t(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23)
		{
			m_flMatVal[0][0] = m00;
			m_flMatVal[0][1] = m01;
			m_flMatVal[0][2] = m02;
			m_flMatVal[0][3] = m03;
			m_flMatVal[1][0] = m10;
			m_flMatVal[1][1] = m11;
			m_flMatVal[1][2] = m12;
			m_flMatVal[1][3] = m13;
			m_flMatVal[2][0] = m20;
			m_flMatVal[2][1] = m21;
			m_flMatVal[2][2] = m22;
			m_flMatVal[2][3] = m23;
		}

		//-----------------------------------------------------------------------------
		// Creates a matrix where the X axis = forward
		// the Y axis = left, and the Z axis = up
		//-----------------------------------------------------------------------------
		void Init(const Vector& xAxis, const Vector& yAxis, const Vector& zAxis, const Vector& vecOrigin)
		{
			m_flMatVal[0][0] = xAxis.x;
			m_flMatVal[0][1] = yAxis.x;
			m_flMatVal[0][2] = zAxis.x;
			m_flMatVal[0][3] = vecOrigin.x;
			m_flMatVal[1][0] = xAxis.y;
			m_flMatVal[1][1] = yAxis.y;
			m_flMatVal[1][2] = zAxis.y;
			m_flMatVal[1][3] = vecOrigin.y;
			m_flMatVal[2][0] = xAxis.z;
			m_flMatVal[2][1] = yAxis.z;
			m_flMatVal[2][2] = zAxis.z;
			m_flMatVal[2][3] = vecOrigin.z;
		}

		//-----------------------------------------------------------------------------
		// Creates a matrix where the X axis = forward
		// the Y axis = left, and the Z axis = up
		//-----------------------------------------------------------------------------
		matrix3x4_t(const Vector& xAxis, const Vector& yAxis, const Vector& zAxis, const Vector& vecOrigin)
		{
			Init(xAxis, yAxis, zAxis, vecOrigin);
		}

		void SetOrigin(Vector const& p)
		{
			m_flMatVal[0][3] = p.x;
			m_flMatVal[1][3] = p.y;
			m_flMatVal[2][3] = p.z;
		}

		void Invalidate()
		{
			for (int i = 0; i < 3; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					m_flMatVal[i][j] = std::numeric_limits<float>::infinity();;
				}
			}
		}

		Vector GetXAxis() const { return at(0); }

		Vector GetYAxis() const { return at(1); }

		Vector GetZAxis() const { return at(2); }

		Vector GetOrigin() const { return at(3); }

		Vector at(int i) const { return Vector{ m_flMatVal[0][i], m_flMatVal[1][i], m_flMatVal[2][i] }; }

		float* operator[](int i) { return m_flMatVal[i]; }

		const float* operator[](int i) const { return m_flMatVal[i]; }

		float* Base() { return &m_flMatVal[0][0]; }

		const float* Base() const { return &m_flMatVal[0][0]; }


		float m_flMatVal[3][4];
	};

	class alignas(16) matrix3x4a_t : public matrix3x4_t
	{
	};

	class VMatrix
	{
	public:
		VMatrix(float m00, float m01, float m02, float m03,
			float m10, float m11, float m12, float m13,
			float m20, float m21, float m22, float m23,
			float m30, float m31, float m32, float m33)
		{
			Init(m00, m01, m02, m03,
				m10, m11, m12, m13,
				m20, m21, m22, m23,
				m30, m31, m32, m33);
		}

		VMatrix(const matrix3x4_t& matrix3x4)
		{
			Init(matrix3x4);
		}

		//-----------------------------------------------------------------------------
		// Creates a matrix where the X axis = forward
		// the Y axis = left, and the Z axis = up
		//-----------------------------------------------------------------------------
		VMatrix(const Vector& xAxis, const Vector& yAxis, const Vector& zAxis)
		{
			Init(xAxis.x, yAxis.x, zAxis.x, 0.0f,
				xAxis.y, yAxis.y, zAxis.y, 0.0f,
				xAxis.z, yAxis.z, zAxis.z, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f);
		}

		void Init(float m00, float m01, float m02, float m03,
			float m10, float m11, float m12, float m13,
			float m20, float m21, float m22, float m23,
			float m30, float m31, float m32, float m33)
		{
			m[0][0] = m00;
			m[0][1] = m01;
			m[0][2] = m02;
			m[0][3] = m03;

			m[1][0] = m10;
			m[1][1] = m11;
			m[1][2] = m12;
			m[1][3] = m13;

			m[2][0] = m20;
			m[2][1] = m21;
			m[2][2] = m22;
			m[2][3] = m23;

			m[3][0] = m30;
			m[3][1] = m31;
			m[3][2] = m32;
			m[3][3] = m33;
		}

		//-----------------------------------------------------------------------------
		// Initialize from a 3x4
		//-----------------------------------------------------------------------------
		void Init(const matrix3x4_t& matrix3x4)
		{
			memcpy(m, matrix3x4.Base(), sizeof(matrix3x4_t));

			m[3][0] = 0.0f;
			m[3][1] = 0.0f;
			m[3][2] = 0.0f;
			m[3][3] = 1.0f;
		}

		float* operator[](int i)
		{
			return m[i];
		}

		const float* operator[](int i) const
		{
			return m[i];
		}

		float* Base()
		{
			return &m[0][0];
		}

		const float* Base() const
		{
			return &m[0][0];
		}

		bool operator==(const VMatrix& src) const
		{
			return memcmp(this, &src, sizeof(VMatrix));
		}

		bool operator!=(const VMatrix& src) const { return !(*this == src); }

		// The matrix.
		float m[4][4];
	};

#if 0
	[[deprecated]]
	auto MatrixGetColumn(const matrix3x4_t& src, int nCol, Vector& pColumn) -> void;
	[[deprecated]]
	auto MatrixPosition(const matrix3x4_t& matrix, Vector& position) -> void;
#endif
}
