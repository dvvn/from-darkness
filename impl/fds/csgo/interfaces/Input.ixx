module;

#include <cstddef>
#include <cstdint>

export module fds.csgo.interfaces.Input;
export import fds.math.vector3;
export import fds.math.qangle;

export namespace fds::csgo
{
    class bf_write;
    class bf_read;
    class CUserCmd;
    class CVerifiedUserCmd;

    class CInput
    {
      public:
        virtual ~CInput() = default;

        std::byte pad0[0xC];                 // 0x00
        bool bTrackIRAvailable;              // 0x0C
        bool bMouseInitialized;              // 0x0D
        bool bMouseActive;                   // 0x0E
        std::byte pad1[0x9A];                // 0x0F
        bool bCameraInThirdPerson;           // 0xA9
        std::byte pad2[0x2];                 // 0xAA
        math::vector3 vecCameraOffset;       // 0xAC
        std::byte pad3[0x38];                // 0xB8
        CUserCmd* pCommands;                 // 0xF0
        CVerifiedUserCmd* pVerifiedCommands; // 0xF4

        CUserCmd* GetUserCmd(int sequence_number);
        CVerifiedUserCmd* GetVerifiedCmd(int sequence_number);
    };
} // namespace fds::csgo
