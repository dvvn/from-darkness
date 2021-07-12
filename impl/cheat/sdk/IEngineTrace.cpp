#include "IEngineTrace.hpp"

using namespace cheat::csgo;
using namespace cheat::utl;

void Ray_t::Init(const Vector& start, const Vector& end)
{
	m_Delta = end - start;

	m_IsSwept = (m_Delta.LengthSqr( ) != 0);

	m_Extents.Init( );

	m_pWorldAxisTransform = nullptr;
	m_IsRay = true;

	// Offset m_Start to be in the center of the box...
	m_StartOffset.Init( );
	m_Start = start;
}

void Ray_t::Init(const Vector& start, const Vector& end, const Vector& mins, const Vector& maxs)
{
	m_Delta = end - start;

	m_pWorldAxisTransform = nullptr;
	m_IsSwept = (m_Delta.LengthSqr( ) != 0);

	m_Extents = maxs - mins;
	m_Extents *= 0.5f;
	m_IsRay = (m_Extents.LengthSqr( ) < 1e-6);

	// Offset m_Start to be in the center of the box...
	m_StartOffset = maxs + mins;
	m_StartOffset *= 0.5f;
	m_Start = start + m_StartOffset;
	m_StartOffset *= -1.0f;
}

Vector Ray_t::InvDelta( ) const
{
	Vector vecInvDelta;
	for (int iAxis = 0; iAxis < 3; ++iAxis)
	{
		if (m_Delta[iAxis] != 0.0f)
		{
			vecInvDelta[iAxis] = 1.0f / m_Delta[iAxis];
		}
		else
		{
			vecInvDelta[iAxis] = FLT_MAX;
		}
	}
	return vecInvDelta;
}

bool CGameTrace::DidHit( ) const
{
	return fraction < 1 || allsolid || startsolid;
}

bool CGameTrace::IsVisible( ) const
{
	return fraction > 0.97f;
}
