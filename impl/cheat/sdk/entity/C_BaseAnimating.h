#pragma once
#include "C_BaseEntity.h"

namespace cheat::csgo
{
	class matrix3x4a_t;
	class Vector;
	class QuaternionAligned;
	class CStudioHdr;
	class Quaternion;
	class CBoneBitList;
	class CIKContext;

	class CAnimationLayer
	{
	public:
		bool m_bClientBlend;          //0x00
		float m_flBlendIn;            //0x04
		void* m_pStudioHdr;           //0x08
		int m_nDispatchedSrc;         //0x0C
		int m_nDispatchedDst;         //0x10
		std::uintptr_t m_iOrder;      //0x14
		std::uintptr_t m_nSequence;   //0x18
		float m_flPrevCycle;          //0x1C
		float m_flWeight;             //0x20
		float m_flWeightDeltaRate;    //0x24
		float m_flPlaybackRate;       //0x28
		float m_flCycle;              //0x2C
		void* m_pOwner;               //0x30
		int m_nInvalidatePhysicsBits; //0x34
	};

	using AnimOverlaysArr_t = std::array<CAnimationLayer, 13>;
	class C_BaseAnimating : public C_BaseEntity
	{
	public:
#if __has_include("../generated/C_BaseAnimating_h")
#include "../generated/C_BaseAnimating_h"
#endif

		void UpdateClientSideAnimation();
		AnimOverlaysArr_t& GetAnimOverlays();
	};
}
