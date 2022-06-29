module;

#include <cstddef>
#include <cstdint>

export module fd.valve.input;
export import fd.valve.user_cmd;

using namespace fd::valve;
using namespace fd::math;

struct input
{
    virtual ~input() = default;

    uint8_t pad0[0xC];                    // 0x00
    bool TrackIRAvailable;                // 0x0C
    bool mouse_initialized;               // 0x0D
    bool mouse_active;                    // 0x0E
    uint8_t pad1[0x9A];                   // 0x0F
    bool camera_in_third_person;          // 0xA9
    uint8_t pad2[0x2];                    // 0xAA
    vector3 camera_offset;                // 0xAC
    uint8_t pad3[0x38];                   // 0xB8
    user_cmd* commands;                   // 0xF0
    verified_user_cmd* verified_commands; // 0xF4

    user_cmd* GetUserCmd(int sequence_number);
    verified_user_cmd* GetVerifiedCmd(int sequence_number);
};

export namespace fd::valve
{
    using ::input;
}
