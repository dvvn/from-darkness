module;

#include <cstdint>

export module cheat.csgo.interfaces:UserCmd;
export import cheat.csgo.math;

export namespace cheat::csgo
{
	class CUserCmd
	{
	public:
		enum buttons :uint32_t
		{
			IN_ATTACK = 1 << 0,
			IN_JUMP = 1 << 1,
			IN_DUCK = 1 << 2,
			IN_FORWARD = 1 << 3,
			IN_BACK = 1 << 4,
			IN_USE = 1 << 5,
			IN_CANCEL = 1 << 6,
			IN_LEFT = 1 << 7,
			IN_RIGHT = 1 << 8,
			IN_MOVELEFT = 1 << 9,
			IN_MOVERIGHT = 1 << 10,
			IN_ATTACK2 = 1 << 11,
			IN_RUN = 1 << 12,
			IN_RELOAD = 1 << 13,
			IN_ALT1 = 1 << 14,
			IN_ALT2 = 1 << 15,
			// Used by client.dll for when scoreboard is held down
			IN_SCORE = 1 << 16,
			// Player is holding the speed key
			IN_SPEED = 1 << 17,
			// Player holding walk key
			IN_WALK = 1 << 18,
			// Zoom key for HUD zoom
			IN_ZOOM = 1 << 19,
			// weapon defines these bits
			IN_WEAPON1 = 1 << 20,
			// weapon defines these bits
			IN_WEAPON2 = 1 << 21,
			IN_BULLRUSH = 1 << 22,
			// grenade 1
			IN_GRENADE1 = 1 << 23,
			// grenade 2
			IN_GRENADE2 = 1 << 24,
			IN_LOOKSPIN = 1 << 25,
		};

		virtual ~CUserCmd( ) = default;
		CUserCmd( );

#if 0
		CRC32_t GetChecksum( ) const
		{
			CRC32_t crc;
			CRC32_Init(&crc);

			CRC32_ProcessBuffer(&crc, &command_number, sizeof(command_number));
			CRC32_ProcessBuffer(&crc, &tick_count, sizeof(tick_count));
			CRC32_ProcessBuffer(&crc, &viewangles, sizeof(viewangles));
			CRC32_ProcessBuffer(&crc, &aimdirection, sizeof(aimdirection));
			CRC32_ProcessBuffer(&crc, &forwardmove, sizeof(forwardmove));
			CRC32_ProcessBuffer(&crc, &sidemove, sizeof(sidemove));
			CRC32_ProcessBuffer(&crc, &upmove, sizeof(upmove));
			CRC32_ProcessBuffer(&crc, &buttons, sizeof(buttons));
			CRC32_ProcessBuffer(&crc, &impulse, sizeof(impulse));
			CRC32_ProcessBuffer(&crc, &weaponselect, sizeof(weaponselect));
			CRC32_ProcessBuffer(&crc, &weaponsubtype, sizeof(weaponsubtype));
			CRC32_ProcessBuffer(&crc, &random_seed, sizeof(random_seed));
			CRC32_ProcessBuffer(&crc, &mousedx, sizeof(mousedx));
			CRC32_ProcessBuffer(&crc, &mousedy, sizeof(mousedy));

			CRC32_Final(&crc);
			return crc;
		}
#endif

		int				iCommandNumber;		// 0x04
		int				iTickCount;			// 0x08
		QAngle			angViewPoint;		// 0x0C
		Vector			vecAimDirection;	// 0x18
		float			flForwardMove;		// 0x24
		float			flSideMove;			// 0x28
		float			flUpMove;			// 0x2C
		int				iButtons;			// 0x30
		std::uint8_t	uImpulse;			// 0x34
		int				iWeaponSelect;		// 0x38
		int				iWeaponSubType;		// 0x3C
		int				iRandomSeed;		// 0x40
		short			sMouseDeltaX;		// 0x44
		short			sMouseDeltaY;		// 0x46
		bool			bHasBeenPredicted;	// 0x48
		Vector			vecHeadAngles;		// 0x4C
		Vector			vecHeadOffset;		// 0x58
	private:
		uint8_t pad_0x4C[0x18]; // 0x4C Current sizeof( usercmd ) =  100  = 0x64
	};

	using crc32_t = unsigned long;

	class CVerifiedUserCmd
	{
	public:
		CUserCmd cmd;
		crc32_t  crc;
	};

}
