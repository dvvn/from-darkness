module;

#include <limits>
#include <tuple>

module cheat.csgo.interfaces.EngineTrace;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

bool CTraceFilterWorldOnly::ShouldHitEntity(IHandleEntity* /*pServerEntity*/, int /*contentsMask*/)
{
	return false;
}

TraceType CTraceFilterWorldOnly::GetTraceType( ) const
{
	return TraceType::TRACE_WORLD_ONLY;
}

//-----

bool CTraceFilterWorldAndPropsOnly::ShouldHitEntity(IHandleEntity*, int)
{
	return false;
}

TraceType CTraceFilterWorldAndPropsOnly::GetTraceType( ) const
{
	return TraceType::TRACE_EVERYTHING;
}

//-----

bool CTraceFilterHitAll::ShouldHitEntity(IHandleEntity*, int)
{
	return true;
}

TraceType CTraceFilterHitAll::GetTraceType( ) const
{
	return TraceType::TRACE_EVERYTHING;
}

//-----

void Ray_t::Init(const Vector& start, const Vector& end)
{
	m_Delta = end - start;

	m_IsSwept = (m_Delta.LengthSqr( ) != 0);

	m_Extents = {};

	m_pWorldAxisTransform = nullptr;
	m_IsRay = true;

	// Offset m_Start to be in the center of the box...
	m_StartOffset = {};
	m_Start = start;
}

void Ray_t::Init(const Vector& start, const Vector& end, const Vector& mins, const Vector& maxs)
{
	m_Delta = end - start;

	m_pWorldAxisTransform = nullptr;
	m_IsSwept = (m_Delta.LengthSqr( ) != 0);

	m_Extents = maxs - mins;
	m_Extents *= 0.5f_fill;
	m_IsRay = (m_Extents.LengthSqr( ) < 1e-6);

	// Offset m_Start to be in the center of the box...
	m_StartOffset = maxs + mins;
	m_StartOffset *= 0.5f_fill;
	m_Start = start + m_StartOffset;
	m_StartOffset *= -1.0f_fill;
}

//constexpr Vector  ec = -1.0f;

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

//-----

bool CBaseTrace::IsDispSurface( ) { return ((dispFlags & DISPSURF_FLAG_SURFACE) != 0); }
bool CBaseTrace::IsDispSurfaceWalkable( ) { return ((dispFlags & DISPSURF_FLAG_WALKABLE) != 0); }
bool CBaseTrace::IsDispSurfaceBuildable( ) { return ((dispFlags & DISPSURF_FLAG_BUILDABLE) != 0); }
bool CBaseTrace::IsDispSurfaceProp1( ) { return ((dispFlags & DISPSURF_FLAG_SURFPROP1) != 0); }
bool CBaseTrace::IsDispSurfaceProp2( ) { return ((dispFlags & DISPSURF_FLAG_SURFPROP2) != 0); }

//-----

bool CGameTrace::DidHit( ) const
{
	return fraction < 1 || allsolid || startsolid;
}

bool CGameTrace::IsVisible( ) const
{
	return fraction > 0.97f;
}

//-----

IEngineTrace* nstd::one_instance_getter<IEngineTrace*>::_Construct( )const
{
	return csgo_modules::engine.find_interface<"EngineTraceClient">( );
}
