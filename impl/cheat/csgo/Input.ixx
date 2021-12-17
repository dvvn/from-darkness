module;

#include <dhooks/helpers.h>

#include <nstd/address.h>

export module cheat.csgo.structs:Input;
export import :UserCmd;
export import cheat.csgo.math;

constexpr auto MULTIPLAYER_BACKUP = 150;

export namespace cheat::csgo
{
	class bf_write;
	class bf_read;

	class CInput
	{
	public:
		char pad_0x00[0x0C];
		bool trackir_available;
		bool mouse_initiated;
		bool mouse_active;
		bool fJoystickAdvancedInit;
		char pad_0x08[0x2C];
		void* pKeys;
		char pad_0x38[0x6C];
		bool fCameraInterceptingMouse;
		bool fCameraInThirdPerson;
		bool fCameraMovingWithMouse;
		Vector vecCameraOffset;
		bool fCameraDistanceMove;
		int nCameraOldX;
		int nCameraOldY;
		int nCameraX;
		int nCameraY;
		bool CameraIsOrthographic;
		Vector angPreviousViewAngles;
		Vector angPreviousViewAnglesTilt;
		float flLastForwardMove;
		int nClearInputState;
		char pad_0xE4[0x8];
		CUserCmd* pCommands;
		CVerifiedUserCmd* pVerifiedCommands;

		CUserCmd* GetUserCmd(int sequence_number, int nSlot = 0)
		{
			return dhooks::_Call_function(&CInput::GetUserCmd, this, 8, nSlot, sequence_number);
		}
		CVerifiedUserCmd* GetVerifiedCmd(int sequence_number)
		{
			CVerifiedUserCmd* verified_commands = nstd::address(this).add(0xF8).deref(1).ptr();
			return &verified_commands[sequence_number % MULTIPLAYER_BACKUP];
		}
	};
}
