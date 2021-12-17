module;

#include <cstdint>

export module cheat.csgo.structs:ClientEntity;
export import :ClientUnknown;
export import :ClientRenderable;
export import :ClientNetworkable;
export import :ClientThinkable;
export import cheat.csgo.math;

export namespace cheat::csgo
{
	struct SpatializationInfo_t;

	class IClientEntity : public IClientUnknown, public IClientRenderable, public IClientNetworkable, public IClientThinkable
	{
	public:
		virtual const Vector& GetAbsOrigin() const = 0;
		virtual const QAngle& GetAbsAngles() const = 0;
		virtual void* GetMouth() = 0;
		virtual bool               GetSoundSpatialization(struct SpatializationInfo_t& info) = 0;
		virtual bool               IsBlurred() = 0;
	};

#pragma pack(push, 1)
	class CCSWeaponInfo
	{
		//xSeeker
	public:
		int8_t  pad0[20];
		int32_t iMaxClip1;
		int8_t  pad1[12];
		int32_t iMaxReservedAmmo;
		int8_t  pad2[96];
		char* szHudName;
		char* szWeaponName;
		int8_t  pad3[56];
		int32_t iWeaponType;
		int8_t  pad4[4];
		int32_t iWeaponPrice;
		int32_t iKillAward;
		int8_t  pad5[20];
		uint8_t bFullAuto;
		int8_t  pad6[3];
		int32_t iDamage;
		float flArmorRatio;
		int32_t iBullets;
		float flPenetration;
		int8_t  pad7[8];
		float flRange;
		float flRangeModifier;
		int8_t  pad8[16];
		uint8_t bHasSilencer;
		int8_t  pad9[15];
		float flSpread;
		float flSpreadAlt;
		int8_t  pad10[76];
		int32_t iRecoilSeed;
		int8_t  pad11[32];
	};
#pragma pack(pop)

	class IWeaponSystem
	{
		virtual void unused0() = 0;
		virtual void unused1() = 0;
	public:
		virtual CCSWeaponInfo* GetWpnData(unsigned ItemDefinitionIndex) = 0;
	};
}
