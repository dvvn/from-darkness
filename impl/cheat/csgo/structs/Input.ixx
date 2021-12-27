module;

#include <cstddef>

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
		std::byte			pad0[0xC];				//0x0000
		bool				bTrackIRAvailable;		//0x000C
		bool				bMouseInitialized;		//0x000D
		bool				bMouseActive;			//0x000E
		std::byte			pad1[0xB2];				//0x000F
		bool				bCameraInThirdPerson;	//0x00C1
		std::byte			pad2[0x2];				//0x00C2
		Vector				vecCameraOffset;		//0x00C4
		std::byte			pad3[0x38];				//0x00D0
		CUserCmd* pCommands;				//0x0108
		CVerifiedUserCmd* pVerifiedCommands;		//0x010C

		CUserCmd* GetUserCmd(int sequence_number, int nSlot = 0);
		CVerifiedUserCmd* GetVerifiedCmd(int sequence_number);
	};
}
