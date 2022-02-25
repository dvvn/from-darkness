module;

#include <cstddef>
#include <cstdint>

export module cheat.csgo.interfaces.Input;
export import cheat.csgo.math;

export namespace cheat::csgo
{
	class bf_write;
	class bf_read;
	class CUserCmd;
	class CVerifiedUserCmd;

	class CInput
	{
	public:

		virtual ~CInput( ) = default;

		char pad_0000[12 - 4]; //0x0000
		bool m_fTrackIRAvailable; //0x000C
		bool m_fMouseInitialized; //0x000D
		bool m_fMouseActive; //0x000E
		bool m_fJoystickAdvancedInit; //0x000F
		char pad_0010[44]; //0x0010
		char* m_pKeys; //0x003C
		char pad_0040[48]; //0x0040
		int32_t m_nCamCommand; //0x0070
		char pad_0074[76]; //0x0074
		bool m_fCameraInterceptingMouse; //0x00C0
		bool m_fCameraInThirdPerson; //0x00C1
		bool m_fCameraMovingWithMouse; //0x00C2
		char pad_00C3[1]; //0x00C3
		Vector* m_vecCameraOffset; //0x00C4
		char pad_00C8[8]; //0x00C8
		bool m_fCameraDistanceMove; //0x00D0
		char pad_00D1[19]; //0x00D1
		bool m_CameraIsOrthographic; //0x00E4
		bool m_CameraIsThirdPersonOverview; //0x00E5
		char pad_00E6[2]; //0x00E6
		QAngle* m_angPreviousViewAngles; //0x00E8
		QAngle* m_angPreviousViewAnglesTilt; //0x00EC
		char pad_00F0[16]; //0x00F0
		float m_flLastForwardMove; //0x0100
		int32_t m_nClearInputState; //0x0104
		CUserCmd* m_pCommands; //0x0108
		CVerifiedUserCmd* m_pVerifiedCommands; //0x010C

		CUserCmd* GetUserCmd(int sequence_number);
		CVerifiedUserCmd* GetVerifiedCmd(int sequence_number);
	};
}
