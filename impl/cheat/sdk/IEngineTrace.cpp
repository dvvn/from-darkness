#include "IEngineTrace.hpp"

using namespace cheat::csgo;
using namespace cheat::utl;

auto Ray_t::Init(const Vector& start, const Vector& end) -> void
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

auto Ray_t::Init(const Vector& start, const Vector& end, const Vector& mins, const Vector& maxs) -> void
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

auto Ray_t::InvDelta( ) const -> Vector
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

auto CGameTrace::DidHit( ) const -> bool
{
	return fraction < 1 || allsolid || startsolid;
}

auto CGameTrace::IsVisible( ) const -> bool
{
	return fraction > 0.97f;
}
