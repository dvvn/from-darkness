#pragma once
#include "cheat/sdk/datamap.hpp"
#include "cheat/sdk/IClientEntity.hpp"
#include "cheat/sdk/IClientRenderable.hpp"
#include "cheat/sdk/UtlVector.hpp"

namespace cheat::csgo
{
	using string_t = const char*;
	// ReSharper disable once CppInconsistentNaming
	using HSCRIPT = uint32_t;
	class IInterpolatedVar;

	class VarMapEntry_t
	{
	public:
		unsigned short type;
		unsigned short m_bNeedsToInterpolate; // Set to false when this var doesn't
		// need Interpolate() called on it anymore.
		void*             data;
		IInterpolatedVar* watcher;
	};

	struct VarMapping_t
	{
		CUtlVector<VarMapEntry_t> m_Entries;
		int                       m_nInterpolatedEntries = 0;
		float                     m_lastInterpolationTime = 0;
	};

	class C_BaseEntity: public IClientEntity, public IClientModelRenderable
	{
	public:
		~C_BaseEntity( ) override =0;

		datamap_t* GetDataDescMap( );
		datamap_t* GetPredictionDescMap( );

#include "../generated/C_BaseEntity_h"
	};
}
