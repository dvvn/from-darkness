module;

#include <cstdint>
#include <cstddef>

export module cheat.csgo.interfaces.WeaponSystem;

export namespace cheat::csgo
{
	enum WeaponType : uint32_t
	{
		WEAPONTYPE_KNIFE = 0,
		WEAPONTYPE_PISTOL = 1,
		WEAPONTYPE_SUBMACHINEGUN = 2,
		WEAPONTYPE_RIFLE = 3,
		WEAPONTYPE_SHOTGUN = 4,
		WEAPONTYPE_SNIPER = 5,
		WEAPONTYPE_MACHINEGUN = 6,
		WEAPONTYPE_C4 = 7,
		WEAPONTYPE_PLACEHOLDER = 8,
		WEAPONTYPE_GRENADE = 9,
		WEAPONTYPE_HEALTHSHOT = 11,
		WEAPONTYPE_FISTS = 12,
		WEAPONTYPE_BREACHCHARGE = 13,
		WEAPONTYPE_BUMPMINE = 14,
		WEAPONTYPE_TABLET = 15,
		WEAPONTYPE_MELEE = 16
	};

	enum ItemDefinitionIndex : uint16_t
	{
		WEAPON_NONE = 0,
		WEAPON_DEAGLE = 1,
		WEAPON_ELITE = 2,
		WEAPON_FIVESEVEN = 3,
		WEAPON_GLOCK = 4,
		WEAPON_AK47 = 7,
		WEAPON_AUG = 8,
		WEAPON_AWP = 9,
		WEAPON_FAMAS = 10,
		WEAPON_G3SG1 = 11,
		WEAPON_GALILAR = 13,
		WEAPON_M249 = 14,
		WEAPON_M4A1 = 16,
		WEAPON_MAC10 = 17,
		WEAPON_P90 = 19,
		WEAPON_ZONE_REPULSOR = 20,
		WEAPON_MP5SD = 23,
		WEAPON_UMP45 = 24,
		WEAPON_XM1014 = 25,
		WEAPON_BIZON = 26,
		WEAPON_MAG7 = 27,
		WEAPON_NEGEV = 28,
		WEAPON_SAWEDOFF = 29,
		WEAPON_TEC9 = 30,
		WEAPON_TASER = 31,
		WEAPON_HKP2000 = 32,
		WEAPON_MP7 = 33,
		WEAPON_MP9 = 34,
		WEAPON_NOVA = 35,
		WEAPON_P250 = 36,
		WEAPON_SHIELD = 37,
		WEAPON_SCAR20 = 38,
		WEAPON_SG556 = 39,
		WEAPON_SSG08 = 40,
		WEAPON_KNIFE_GG = 41,
		WEAPON_KNIFE = 42,
		WEAPON_FLASHBANG = 43,
		WEAPON_HEGRENADE = 44,
		WEAPON_SMOKEGRENADE = 45,
		WEAPON_MOLOTOV = 46,
		WEAPON_DECOY = 47,
		WEAPON_INCGRENADE = 48,
		WEAPON_C4 = 49,
		WEAPON_HEALTHSHOT = 57,
		WEAPON_KNIFE_T = 59,
		WEAPON_M4A1_SILENCER = 60,
		WEAPON_USP_SILENCER = 61,
		WEAPON_CZ75A = 63,
		WEAPON_REVOLVER = 64,
		WEAPON_TAGRENADE = 68,
		WEAPON_FISTS = 69,
		WEAPON_BREACHCHARGE = 70,
		WEAPON_TABLET = 72,
		WEAPON_MELEE = 74,
		WEAPON_AXE = 75,
		WEAPON_HAMMER = 76,
		WEAPON_SPANNER = 78,
		WEAPON_KNIFE_GHOST = 80,
		WEAPON_FIREBOMB = 81,
		WEAPON_DIVERSION = 82,
		WEAPON_FRAG_GRENADE = 83,
		WEAPON_SNOWBALL = 84,
		WEAPON_BUMPMINE = 85,
		WEAPON_KNIFE_BAYONET = 500,
		WEAPON_KNIFE_CSS = 503,
		WEAPON_KNIFE_FLIP = 505,
		WEAPON_KNIFE_GUT = 506,
		WEAPON_KNIFE_KARAMBIT = 507,
		WEAPON_KNIFE_M9_BAYONET = 508,
		WEAPON_KNIFE_TACTICAL = 509,
		WEAPON_KNIFE_FALCHION = 512,
		WEAPON_KNIFE_SURVIVAL_BOWIE = 514,
		WEAPON_KNIFE_BUTTERFLY = 515,
		WEAPON_KNIFE_PUSH = 516,
		WEAPON_KNIFE_CORD = 517,
		WEAPON_KNIFE_CANIS = 518,
		WEAPON_KNIFE_URSUS = 519,
		WEAPON_KNIFE_GYPSY_JACKKNIFE = 520,
		WEAPON_KNIFE_OUTDOOR = 521,
		WEAPON_KNIFE_STILETTO = 522,
		WEAPON_KNIFE_WIDOWMAKER = 523,
		WEAPON_KNIFE_SKELETON = 525,
		GLOVE_STUDDED_BROKENFANG = 4725,
		GLOVE_STUDDED_BLOODHOUND = 5027,
		GLOVE_T = 5028,
		GLOVE_CT = 5029,
		GLOVE_SPORTY = 5030,
		GLOVE_SLICK = 5031,
		GLOVE_LEATHER_HANDWRAPS = 5032,
		GLOVE_MOTORCYCLE = 5033,
		GLOVE_SPECIALIST = 5034,
		GLOVE_STUDDED_HYDRA = 5035,
		SPECIAL_AGENT_BLUEBERRIES_BUCKSHOT = 4619,
		SPECIAL_AGENT_TWO_TIMES_MCCOY_TACP = 4680,
		SPECIAL_AGENT_COMMANDOR_MAE_JAMISON = 4711,
		SPECIAL_AGENT_1ST_LIEUTENANT_FARLOW,
		SPECIAL_AGENT_JOHN_KASK,
		SPECIAL_AGENT_BIO_HAZ_SPECIALIST,
		SPECIAL_AGENT_SERGEANT_BOMBSON,
		SPECIAL_AGENT_CHEM_HAZ_SPECIALIST,
		SPECIAL_AGENT_REZAN_THE_REDSHIRT = 4718,
		SPECIAL_AGENT_SIR_BLOODY_MIAMI_DARRYL = 4726,
		SPECIAL_AGENT_SAFECRACKER_VOLTZMANN,
		SPECIAL_AGENT_LITTLE_KEV,
		SPECIAL_AGENT_GETAWAY_SALLY = 4730,
		SPECIAL_AGENT_NUMBER_K = 4732,
		SPECIAL_AGENT_SIR_BLOODY_SILENT_DARRYL = 4733,
		SPECIAL_AGENT_SIR_BLOODY_SKULLHEAD_DARRYL,
		SPECIAL_AGENT_SIR_BLOODY_DARRYL_ROYALE,
		SPECIAL_AGENT_SIR_BLOODY_LOUDMOUTH_DARRYL,
		SPECIAL_AGENT_T = 5036,
		SPECIAL_AGENT_CT = 5037,
		SPECIAL_AGENT_GROUND_REBEL = 5105,
		SPECIAL_AGENT_OSIRIS,
		SPECIAL_AGENT_SHAHMAT,
		SPECIAL_AGENT_MUHLIK,
		SPECIAL_AGENT_SOLDIER = 5205,
		SPECIAL_AGENT_ENFORCER,
		SPECIAL_AGENT_SLINGSHOT,
		SPECIAL_AGENT_STREET_SOLDIER,
		SPECIAL_AGENT_OPERATOR = 5305,
		SPECIAL_AGENT_MARKUS_DELROW,
		SPECIAL_AGENT_MICHAEL_SYFERS,
		SPECIAL_AGENT_AVA,
		SPECIAL_AGENT_3RD_COMMANDO_COMPANY = 5400,
		SPECIAL_AGENT_SEAL_TEAM_6_SOLDIER,
		SPECIAL_AGENT_BUCKSHOT,
		SPECIAL_AGENT_TWO_TIMES_MCCOY_USAF,
		SPECIAL_AGENT_RICKSAW,
		SPECIAL_AGENT_DRAGOMIR = 5500,
		SPECIAL_AGENT_MAXIMUS,
		SPECIAL_AGENT_REZAN_THE_READY,
		SPECIAL_AGENT_BLACKWOLF = 5503,
		SPECIAL_AGENT_THE_DOCTOR,
		SPECIAL_AGENT_DRAGOMIR_FOOTSOLDIERS,
		SPECIAL_AGENT_B_SQUADRON_OFFICER = 5601
	};

	class CCSWeaponInfo
	{
	public:


		std::byte pad0[0x14];			// 0x0000
		int iMaxClip1;					// 0x0014
		int iMaxClip2;					// 0x0018
		int iDefaultClip1;				// 0x001C
		int iDefaultClip2;				// 0x0020
		int iPrimaryMaxReserveAmmo;		// 0x0024
		int iSecondaryMaxReserveAmmo;	// 0x0028
		const char* szWorldModel;		// 0x002C
		const char* szViewModel;		// 0x0030
		const char* szDroppedModel;		// 0x0034
		std::byte pad1[0x50];			// 0x0038
		const char* szHudName;			// 0x0088
		const char* szWeaponName;		// 0x008C
		std::byte pad2[0x2];			// 0x0090
		bool bIsMeleeWeapon;			// 0x0092
		std::byte pad3[0x9];			// 0x0093
		float flWeaponWeight;			// 0x009C
		std::byte pad4[0x4];			// 0x00A0
		int iSlot;						// 0x00A4
		int iPosition;					// 0x00A8
		std::byte pad5[0x1C];			// 0x00AC
		WeaponType nWeaponType;				// 0x00C8
		std::byte pad6[0x4];			// 0x00CC
		int iWeaponPrice;				// 0x00D0
		int iKillAward;					// 0x00D4
		const char* szAnimationPrefix;	// 0x00D8
		float flCycleTime;				// 0x00DC
		float flCycleTimeAlt;			// 0x00E0
		std::byte pad8[0x8];			// 0x00E4
		bool bFullAuto;					// 0x00EC
		std::byte pad9[0x3];			// 0x00ED
		int iDamage;					// 0x00F0
		float flArmorRatio;				// 0x00F4
		float flHeadShotMultiplier;		// 0x00F8
		int iBullets;					// 0x00FC
		float flPenetration;			// 0x0100
		std::byte pad10[0x8];			// 0x0104
		float flRange;					// 0x010C
		float flRangeModifier;			// 0x0110
		float flThrowVelocity;			// 0x0114
		std::byte pad11[0xC];			// 0x0118
		bool bHasSilencer;				// 0x0124
		std::byte pad12[0xF];			// 0x0125
		float flMaxSpeed[2];			// 0x0134
		std::byte pad13[0x4];			// 0x013C
		float flSpread[2];				// 0x0140
		float flInaccuracyCrouch[2];	// 0x0148
		float flInaccuracyStand[2];		// 0x0150
		std::byte pad14[0x8];			// 0x0158
		float flInaccuracyJump[2];		// 0x0160
		float flInaccuracyLand[2];		// 0x0168
		float flInaccuracyLadder[2];	// 0x0170
		float flInaccuracyFire[2];		// 0x0178
		float flInaccuracyMove[2];		// 0x0180
		float flInaccuracyReload;		// 0x0188
		int iRecoilSeed;				// 0x018C
		float flRecoilAngle[2];			// 0x0190
		float flRecoilAngleVariance[2];	// 0x0198
		float flRecoilMagnitude[2];		// 0x01A0
		float flRecoilMagnitudeVariance[2]; // 0x01A8
		int iSpreadSeed;				// 0x01B0

		/*bool IsGun( )
		{
			switch (this->nWeaponType)
			{
			case WEAPONTYPE_PISTOL:
			case WEAPONTYPE_SUBMACHINEGUN:
			case WEAPONTYPE_RIFLE:
			case WEAPONTYPE_SHOTGUN:
			case WEAPONTYPE_SNIPER:
			case WEAPONTYPE_MACHINEGUN:
				return true;
			}

			return false;
		}*/
	};

	class IWeaponSystem
	{
	public:
		virtual ~IWeaponSystem( ) = default;
		virtual void unknown( ) = 0;

		virtual CCSWeaponInfo* GetWpnData(ItemDefinitionIndex index) = 0;
	};
}