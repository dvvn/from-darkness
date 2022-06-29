module;

#include <cstdint>

export module fd.valve.user_cmd;
export import fd.math.qangle;
export import fd.math.vector3;

using namespace fd::math;

struct user_cmd
{
    enum button_type : uint32_t
    {
        IN_ATTACK    = 1 << 0,
        IN_JUMP      = 1 << 1,
        IN_DUCK      = 1 << 2,
        IN_FORWARD   = 1 << 3,
        IN_BACK      = 1 << 4,
        IN_USE       = 1 << 5,
        IN_CANCEL    = 1 << 6,
        IN_LEFT      = 1 << 7,
        IN_RIGHT     = 1 << 8,
        IN_MOVELEFT  = 1 << 9,
        IN_MOVERIGHT = 1 << 10,
        IN_ATTACK2   = 1 << 11,
        IN_RUN       = 1 << 12,
        IN_RELOAD    = 1 << 13,
        IN_ALT1      = 1 << 14,
        IN_ALT2      = 1 << 15,
        // Used by client.dll for when scoreboard is held down
        IN_SCORE     = 1 << 16,
        // Player is holding the speed key
        IN_SPEED     = 1 << 17,
        // Player holding walk key
        IN_WALK      = 1 << 18,
        // Zoom key for HUD zoom
        IN_ZOOM      = 1 << 19,
        // weapon defines these bits
        IN_WEAPON1   = 1 << 20,
        // weapon defines these bits
        IN_WEAPON2   = 1 << 21,
        IN_BULLRUSH  = 1 << 22,
        // grenade 1
        IN_GRENADE1  = 1 << 23,
        // grenade 2
        IN_GRENADE2  = 1 << 24,
        IN_LOOKSPIN  = 1 << 25,
    };

    virtual ~user_cmd() = default;
    user_cmd();

    int command_number;      // 0x04
    int tick_count;          // 0x08
    qangle view_point;       // 0x0C
    vector3 aim_direction;   // 0x18
    float forward_move;      // 0x24
    float side_move;         // 0x28
    float up_move;           // 0x2C
    button_type Buttons;     // 0x30
    uint8_t impulse;         // 0x34
    int weapon_select;       // 0x38
    int weapon_sub_type;     // 0x3C
    int random_seed;         // 0x40
    short mouse_delta_x;     // 0x44
    short mouse_delta_y;     // 0x46
    bool has_been_predicted; // 0x48
    vector3 head_angles;     // 0x4C
    vector3 head_offset;     // 0x58
  private:
    uint8_t pad_0x4C[0x18]; // 0x4C Current sizeof( usercmd ) =  100  = 0x64
};

struct verified_user_cmd
{
    user_cmd cmd;
    /* crc32_t */ unsigned long crc;
};

export namespace fd::valve
{
    using ::user_cmd;
    using ::verified_user_cmd;
} // namespace fd::valve
