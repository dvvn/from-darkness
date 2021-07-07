#pragma once
#include "C_BaseEntity.h"

namespace cheat::csgo
{
	class ICustomMaterial;

	class CCustomMaterialOwner
	{
	public:
		virtual ~CCustomMaterialOwner( ) =0;
		// either replaces and existing material (releasing the old one), or adds one to the vector
		virtual auto SetCustomMaterial(ICustomMaterial* pCustomMaterial, int nIndex = 0) -> void;
		virtual auto OnCustomMaterialsUpdated( ) -> void =0;
		virtual auto DuplicateCustomMaterialsToOther(CCustomMaterialOwner* pOther) const -> void =0;

		// Pointers to custom materials owned by the mat system for this entity. Index
		// in this vector corresponds to the model material index to override with the custom material.
		CUtlVector<ICustomMaterial*> m_ppCustomMaterials;
	};

	class C_BaseAnimating: public C_BaseEntity, public CCustomMaterialOwner
	{
	public:
	};
}
