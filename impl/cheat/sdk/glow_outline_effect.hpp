#pragma once
#include "UtlVector.hpp"

namespace cheat::csgo
{
	class GlowObjectDefinition_t
	{
	public:
		GlowObjectDefinition_t( ) { memset(this, 0, sizeof(*this)); }

		class IClientEntity* m_pEntity; //0x0000
		union
		{
			utl::Vector m_vGlowColor; //0x0004
			struct
			{
				float m_flRed;   //0x0004
				float m_flGreen; //0x0008
				float m_flBlue;  //0x000C
			};
		};
		float   m_flAlpha;               //0x0010
		uint8_t pad_0014[4];             //0x0014
		float   m_flSomeFloat;           //0x0018
		uint8_t pad_001C[4];             //0x001C
		float   m_flAnotherFloat;        //0x0020
		bool    m_bRenderWhenOccluded;   //0x0024
		bool    m_bRenderWhenUnoccluded; //0x0025
		bool    m_bFullBloomRender;      //0x0026
		uint8_t pad_0027[5];             //0x0027
		int32_t m_nGlowStyle;            //0x002C
		int32_t m_nSplitScreenSlot;      //0x0030
		int32_t m_nNextFreeSlot;         //0x0034

		auto IsUnused( ) const -> bool { return m_nNextFreeSlot != ENTRY_IN_USE; }

		static constexpr int END_OF_FREE_LIST = -1;
		static constexpr int ENTRY_IN_USE = -2;
	}; //Size: 0x0038 (56)

	class CGlowObjectManager
	{
	public:
#if 0
    int RegisterGlowObject(IClientEntity *pEntity, const utl::Vector &vGlowColor, float flGlowAlpha, bool bRenderWhenOccluded, bool bRenderWhenUnoccluded, int nSplitScreenSlot)
    {
        int nIndex;
        if(m_nFirstFreeSlot == GlowObjectDefinition_t::END_OF_FREE_LIST) {
            nIndex = m_GlowObjectDefinitions.AddToTail();
        } else {
            nIndex = m_nFirstFreeSlot;
            m_nFirstFreeSlot = m_GlowObjectDefinitions[nIndex].m_nNextFreeSlot;
        }

        m_GlowObjectDefinitions[nIndex].m_pEntity = pEntity;
        m_GlowObjectDefinitions[nIndex].m_vGlowColor = vGlowColor;
        m_GlowObjectDefinitions[nIndex].m_flAlpha = flGlowAlpha;
        m_GlowObjectDefinitions[nIndex].m_bRenderWhenOccluded = bRenderWhenOccluded;
        m_GlowObjectDefinitions[nIndex].m_bRenderWhenUnoccluded = bRenderWhenUnoccluded;
        m_GlowObjectDefinitions[nIndex].m_nSplitScreenSlot = nSplitScreenSlot;
        m_GlowObjectDefinitions[nIndex].m_nNextFreeSlot = GlowObjectDefinition_t::ENTRY_IN_USE;

        return nIndex;
    }
#endif
		auto UnregisterGlowObject(int nGlowObjectHandle) -> void
		{
			m_GlowObjectDefinitions[nGlowObjectHandle].m_nNextFreeSlot = m_nFirstFreeSlot;
			m_GlowObjectDefinitions[nGlowObjectHandle].m_pEntity = 0;
			m_nFirstFreeSlot = nGlowObjectHandle;
		}

		auto SetEntity(int nGlowObjectHandle, IClientEntity* pEntity) -> void
		{
			m_GlowObjectDefinitions[nGlowObjectHandle].m_pEntity = pEntity;
		}

		auto SetColor(int nGlowObjectHandle, const utl::Vector& vGlowColor) -> void
		{
			m_GlowObjectDefinitions[nGlowObjectHandle].m_vGlowColor = vGlowColor;
		}

		auto SetAlpha(int nGlowObjectHandle, float flAlpha) -> void
		{
			m_GlowObjectDefinitions[nGlowObjectHandle].m_flAlpha = flAlpha;
		}

		auto SetRenderFlags(int nGlowObjectHandle, bool bRenderWhenOccluded, bool bRenderWhenUnoccluded) -> void
		{
			m_GlowObjectDefinitions[nGlowObjectHandle].m_bRenderWhenOccluded = bRenderWhenOccluded;
			m_GlowObjectDefinitions[nGlowObjectHandle].m_bRenderWhenUnoccluded = bRenderWhenUnoccluded;
		}

		auto IsRenderingWhenOccluded(int nGlowObjectHandle) const -> bool
		{
			return m_GlowObjectDefinitions[nGlowObjectHandle].m_bRenderWhenOccluded;
		}

		auto IsRenderingWhenUnoccluded(int nGlowObjectHandle) const -> bool
		{
			return m_GlowObjectDefinitions[nGlowObjectHandle].m_bRenderWhenUnoccluded;
		}

		auto HasGlowEffect(IClientEntity* pEntity) const -> bool
		{
			for (int i = 0; i < m_GlowObjectDefinitions.size( ); ++i)
			{
				if (!m_GlowObjectDefinitions[i].IsUnused( ) && m_GlowObjectDefinitions[i].m_pEntity == pEntity)
				{
					return true;
				}
			}

			return false;
		}

		CUtlVector<GlowObjectDefinition_t> m_GlowObjectDefinitions; //0x0000
		int                                m_nFirstFreeSlot;        //0x000C
	};
}
