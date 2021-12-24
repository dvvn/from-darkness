module;

#include <limits>
#include <tuple>

#include "helpers.h"

export module cheat.csgo.math.Vmatrix;

export import cheat.csgo.math.Vector;
export import cheat.csgo.math.array_view;

template<typename T>
concept array_unpackable1 = requires(T && val)
{
	_Array_unpack(std::forward<T>(val));
};

namespace cheat::csgo
{
	export using matrix3x4_data = array_view<Vector, 4, std::numeric_limits<float>::infinity( )>;

	constexpr matrix3x4_data construct_matrix3x4(const Vector& x, const Vector& y, const Vector& z, const Vector& origin)
	{
		return {
	  x[0] ,y[0] ,z[0] ,origin[0]
	 ,x[1] ,y[1] ,z[1] ,origin[1]
	 ,x[2] ,y[2] ,z[2] ,origin[2]
		};
	}
}

export namespace cheat::csgo
{


	using matrix3x4_item0 = Array_view_item<float, 4, 0>;
	struct matrix3x4_asix_base
	{

		/*m_flMatVal[0][0] = xAxis.x; <---x
		m_flMatVal[0][1] = yAxis.x;
		m_flMatVal[0][2] = zAxis.x;
		m_flMatVal[0][3] = vecOrigin.x;
		m_flMatVal[1][0] = xAxis.y; <---y
		m_flMatVal[1][1] = yAxis.y;
		m_flMatVal[1][2] = zAxis.y;
		m_flMatVal[1][3] = vecOrigin.y;
		m_flMatVal[2][0] = xAxis.z; <---z
		m_flMatVal[2][1] = yAxis.z;
		m_flMatVal[2][2] = zAxis.z;
		m_flMatVal[2][3] = vecOrigin.z;*/

		matrix3x4_item0 x, y, z;

		//constexpr matrix3x4_asix_base(const Vector& vec) :x(vec.x), y(vec.y), z(vec.z) { }
	};



	//todo [[no_unique_address]] test!!!
	template<size_t Idx>
	class matrix3x4_asix :public matrix3x4_asix_base
	{
		[[no_unique_address]] std::array<float, Idx> pad_;
	};


	/*template<>
	class matrix3x4_asix<0> :public matrix3x4_asix_base
	{
	};*/

	struct matrix3x4_base
	{
		union
		{
			matrix3x4_data _Data;
			matrix3x4_asix<0> x;
			matrix3x4_asix<1> y;
			matrix3x4_asix<2> z;
			matrix3x4_asix<3> origin;
		};

		constexpr matrix3x4_base(const Vector& x, const Vector& y, const Vector& z, const Vector& origin) :_Data(construct_matrix3x4(x, y, z, origin)) { }

		constexpr bool operator==(const matrix3x4_base& other)const { return _Data == other._Data; }
		constexpr bool operator!=(const matrix3x4_base& other)const { return _Data != other._Data; }
	};

	class matrix3x4_t
	{
	public:
		matrix3x4_t( ) = default;

		matrix3x4_t(float m00, float m01, float m02, float m03,
					float m10, float m11, float m12, float m13,
					float m20, float m21, float m22, float m23);

		//-----------------------------------------------------------------------------
		// Creates a matrix where the X axis = forward
		// the Y axis = left, and the Z axis = up
		//-----------------------------------------------------------------------------
		void Init(const Vector& xAxis, const Vector& yAxis, const Vector& zAxis, const Vector& vecOrigin);

		//-----------------------------------------------------------------------------
		// Creates a matrix where the X axis = forward
		// the Y axis = left, and the Z axis = up
		//-----------------------------------------------------------------------------
		matrix3x4_t(const Vector& xAxis, const Vector& yAxis, const Vector& zAxis, const Vector& vecOrigin);

		void SetOrigin(Vector const& p);

		void Invalidate( );

		Vector GetXAxis( ) const;
		Vector GetYAxis( ) const;
		Vector GetZAxis( ) const;
		Vector GetOrigin( ) const;

		Vector at(int i) const;

		float* operator[](int i);
		const float* operator[](int i) const;
		float* Base( );
		const float* Base( ) const;

		float m_flMatVal[3][4];
	};

	class alignas(16) matrix3x4a_t : public matrix3x4_t
	{
	};

	class VMatrix
	{
	public:
		VMatrix( ) = default;
		VMatrix(float m00, float m01, float m02, float m03,
				float m10, float m11, float m12, float m13,
				float m20, float m21, float m22, float m23,
				float m30, float m31, float m32, float m33);

		// Creates a matrix where the X axis = forward
		// the Y axis = left, and the Z axis = up
		VMatrix(const Vector& forward, const Vector& left, const Vector& up);

		// Construct from a 3x4 matrix
		VMatrix(const matrix3x4_t& matrix3x4);

		// Set the values in the matrix.
		void Init(float m00, float m01, float m02, float m03,
				  float m10, float m11, float m12, float m13,
				  float m20, float m21, float m22, float m23,
				  float m30, float m31, float m32, float m33);

		// Initialize from a 3x4
		void Init(const matrix3x4_t& matrix3x4);

		// array access
		float* operator[](int i);

		const float* operator[](int i) const;

		// Get a pointer to m[0][0]
		float* Base( );

		const float* Base( ) const;

		void SetLeft(const Vector& vLeft);
		void SetUp(const Vector& vUp);
		void SetForward(const Vector& vForward);

		void GetBasisVectors(Vector& vForward, Vector& vLeft, Vector& vUp) const;
		void SetBasisVectors(const Vector& vForward, const Vector& vLeft, const Vector& vUp);

		// Get/Set the translation.
		Vector& GetTranslation(Vector& vTrans) const;
		void SetTranslation(const Vector& vTrans);

		void PreTranslate(const Vector& vTrans);
		void PostTranslate(const Vector& vTrans);

		matrix3x4_t& As3x4( );
		const matrix3x4_t& As3x4( ) const;
		void CopyFrom3x4(const matrix3x4_t& m3x4);
		void Set3x4(matrix3x4_t& matrix3x4) const;

		bool operator==(const VMatrix& src) const;
		bool operator!=(const VMatrix& src) const;

		// Access the basis vectors.
		Vector GetLeft( ) const;
		Vector GetUp( ) const;
		Vector GetForward( ) const;
		Vector GetTranslation( ) const;

		// Matrix->vector operations.
	public:
		// Multiply by a 3D vector (same as operator*).
		void V3Mul(const Vector& vIn, Vector& vOut) const;

		// Multiply by a 4D vector.
		//void  V4Mul( const Vector4D &vIn, Vector4D &vOut ) const;

		// Applies the rotation (ignores translation in the matrix). (This just calls VMul3x3).
		Vector ApplyRotation(const Vector& vVec) const;

		// Multiply by a vector (divides by w, assumes input w is 1).
		Vector operator*(const Vector& vVec) const;

		// Multiply by the upper 3x3 part of the matrix (ie: only apply rotation).
		Vector VMul3x3(const Vector& vVec) const;

		// Apply the inverse (transposed) rotation (only works on pure rotation matrix)
		Vector VMul3x3Transpose(const Vector& vVec) const;

		// Multiply by the upper 3 rows.
		Vector VMul4x3(const Vector& vVec) const;

		// Apply the inverse (transposed) transformation (only works on pure rotation/translation)
		Vector VMul4x3Transpose(const Vector& vVec) const;

		// Matrix->plane operations.
		//public:
		// Transform the plane. The matrix can only contain translation and rotation.
		//void  TransformPlane( const VPlane &inPlane, VPlane &outPlane ) const;

		// Just calls TransformPlane and returns the result.
		//VPlane  operator*(const VPlane &thePlane) const;

		// Matrix->matrix operations.
	public:
		//auto operator=(const VMatrix& mOther) -> VMatrix&;

		// Add two matrices.
		const VMatrix& operator+=(const VMatrix& other);

		// Add/Subtract two matrices.
		VMatrix operator+(const VMatrix& other) const;
		VMatrix operator-(const VMatrix& other) const;

		// Negation.
		VMatrix operator-( ) const;

		// Return inverse matrix. Be careful because the results are undefined 
		// if the matrix doesn't have an inverse (ie: InverseGeneral returns false).
		//auto operator~( ) const -> VMatrix;

		// Matrix operations.
	public:
		// Set to identity.
		void Identity( );
		bool IsIdentity( ) const;
	public:
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
