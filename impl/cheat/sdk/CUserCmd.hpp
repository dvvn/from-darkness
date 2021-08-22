#pragma once

namespace cheat::csgo
{
	struct cmd_buttons final
	{
		enum value_type :uint32_t
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

		NSTD_ENUM_STRUCT_BITFLAG(cmd_buttons);
	};
	class CUserCmd
	{
	public:
		CUserCmd( )
		{
			memset(this, 0, sizeof(CUserCmd));
		};

		virtual ~CUserCmd( )
		{
		}

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

		int         command_number;     // 0x04 For matching server and client commands for debugging
		int         tick_count;         // 0x08 the tick the client created this command
		QAngle      view_angles;        // 0x0C Player instantaneous view angles.
		Vector      aim_direction;      // 0x18
		float       forward_move;       // 0x24
		float       side_move;          // 0x28
		float       up_move;            // 0x2C
		cmd_buttons buttons;            // 0x30 Attack button states
		char        impulse;            // 0x34
		int         weapon_select;      // 0x38 Current weapon id
		int         weapon_subtype;     // 0x3C
		int         random_seed;        // 0x40 For shared random functions
		short       mousedx;            // 0x44 mouse accum in x from create move
		short       mousedy;            // 0x46 mouse accum in y from create move
		bool        has_been_predicted; // 0x48 Client only, tracks whether we've predicted this command at least once
	private:
		std::byte pad_0x4C[0x18]; // 0x4C Current sizeof( usercmd ) =  100  = 0x64
	};

	using crc32_t = /*unsigned long*/uint32_t;

	class CVerifiedUserCmd
	{
	public:
		CUserCmd cmd;
		crc32_t  crc;
	};
}
