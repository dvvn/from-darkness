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
		virtual void SetCustomMaterial(ICustomMaterial* pCustomMaterial, int nIndex = 0);
		virtual void OnCustomMaterialsUpdated( ) =0;
		virtual void DuplicateCustomMaterialsToOther(CCustomMaterialOwner* pOther) const =0;

		// Pointers to custom materials owned by the mat system for this entity. Index
		// in this vector corresponds to the model material index to override with the custom material.
		CUtlVector<ICustomMaterial*> m_ppCustomMaterials;
	};

	class C_BaseAnimating: public C_BaseEntity, public CCustomMaterialOwner
	{
	public:
#include "../generated/C_BaseAnimating_h"

		void UpdateClientSideAnimation( );
	};
}
