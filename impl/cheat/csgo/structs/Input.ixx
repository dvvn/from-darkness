export module cheat.csgo.structs.Input;
import cheat.csgo.math.Vector;


export namespace cheat::csgo
{
	class bf_write;
	class bf_read;
	class CUserCmd;
	class CVerifiedUserCmd;

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

		CUserCmd* GetUserCmd(int sequence_number, int nSlot = 0);
		CVerifiedUserCmd* GetVerifiedCmd(int sequence_number);
	};
}
