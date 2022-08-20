module;

#include <cstdint>

export module fd.valve.prediction;
export import fd.valve.base_handle;
export import fd.math.vector3;
export import fd.math.qangle;
export import fd.valve.vector;

using namespace fd::valve;
using namespace fd::math;

struct move_data
{
    bool FirstRunOfFunctions : 1;
    bool GameCodeMovedPlayer : 1;
    int PlayerHandle;           // edict index on server, client entity handle on client=
    int ImpulseCommand;         // Impulse command issued.
    vector3 m_vecViewAngles;    // Command view angles (local space)
    vector3 m_vecAbsViewAngles; // Command view angles (world space)
    int Buttons;                // Attack buttons.
    int OldButtons;             // From host_client->oldbuttons;
    float ForwardMove;
    float SideMove;
    float UpMove;
    float MaxSpeed;
    float ClientMaxSpeed;
    vector3 m_vecVelocity; // edict::velocity        // Current movement direction.
    vector3 m_vecAngles;   // edict::angles
    vector3 m_vecOldAngles;
    float m_outStepHeight; // how much you climbed this move
    vector3 m_outWishVel;  // This is where you tried
    vector3 m_outJumpVel;  // This is your jump velocity
    vector3 m_vecConstraintCenter;
    float ConstraintRadius;
    float ConstraintWidth;
    float ConstraintSpeedFactor;
    float Unknown[5];
    vector3 m_vecAbsOrigin;
};

class client_entity;

class move_helper
{
  public:
    virtual void _vpad()                      = 0;
    virtual void SetHost(client_entity* host) = 0;
};

struct prediction
{
    virtual ~prediction() = default;

    uintptr_t hLastGround;            // 0x0004
    bool InPrediction;                // 0x0008
    bool IsFirstTimePredicted;        // 0x0009
    bool EnginePaused;                // 0x000A
    bool OldCLPredictValue;           // 0x000B
    int iPreviousStartFrame;          // 0x000C
    int nIncomingPacketNumber;        // 0x0010
    float flLastServerWorldTimeStamp; // 0x0014

    struct
    {
        bool IsFirstTimePredicted;                                    // 0x0018
        uint8_t pad0[0x3];                                            // 0x0019
        int nCommandsPredicted;                                       // 0x001C
        int nServerCommandsAcknowledged;                              // 0x0020
        int iPreviousAckHadErrors;                                    // 0x0024
        float flIdealPitch;                                           // 0x0028
        int iLastCommandAcknowledged;                                 // 0x002C
        bool PreviousAckErrorTriggersFullLatchReset;                  // 0x0030
        vector<base_handle> vecEntitiesWithPredictionErrorsInLastAck; // 0x0031
        bool PerformedTickShift;                                      // 0x0045
                                                                      //-
    } Split[1];                                                       // 0x0018

#if 0
		bool InPrediction( )
		{
			typedef bool(__thiscall* oInPrediction)(void*);
			return CallVFunction<oInPrediction>(this, 14)(this);
		}

		void RunCommand(base_player* player, user_cmd* ucmd, move_helper* moveHelper)
		{
			typedef void(__thiscall* oRunCommand)(void*, base_player*, user_cmd*, move_helper*);
			return CallVFunction<oRunCommand>(this, 19)(this, player, ucmd, moveHelper);
		}

		void SetupMove(base_player* player, user_cmd* ucmd, move_helper* moveHelper, move_data* pMoveData)
		{
			typedef void(__thiscall* oSetupMove)(void*, base_player*, user_cmd*, move_helper*, void*);
			return CallVFunction<oSetupMove>(this, 20)(this, player, ucmd, moveHelper, pMoveData);
		}

		void FinishMove(base_player* player, user_cmd* ucmd, void* pMoveData)
		{
			typedef void(__thiscall* oFinishMove)(void*, base_player*, user_cmd*, void*);
			return CallVFunction<oFinishMove>(this, 21)(this, player, ucmd, pMoveData);
		}
#endif

    virtual void Init(void)     = 0; // 1
    virtual void Shutdown(void) = 0; // 2

    // Run prediction
    virtual void Update(int startframe,            // World update ( un-modded ) most recently received
                        bool validframe,           // Is frame data valid
                        int incoming_acknowledged, // Last command acknowledged to have been run by server (un-modded)
                        int outgoing_command       // Last command (most recent) sent to server (un-modded)
                        ) = 0;                     // 3

    // We are about to get a network update from the server.  We know the update #, so we can pull any
    //  data purely predicted on the client side and transfer it to the new from data state.
    virtual void PreEntityPacketReceived(int commands_acknowledged, int current_world_update_packet, int server_ticks_elapsed) = 0; // 4
    virtual void PostEntityPacketReceived(void)                                                                                = 0; // 5
    virtual void PostNetworkDataReceived(int commands_acknowledged)                                                            = 0; // 6

    virtual void OnReceivedUncompressedPacket(void) = 0; // 7

    // The engine needs to be able to access a few predicted values
    virtual void GetViewOrigin(vector3& org)     = 0; // 8
    virtual void SetViewOrigin(vector3& org)     = 0; // 9
    virtual void GetViewAngles(qangle& ang)      = 0; // 10
    virtual void SetViewAngles(qangle& ang)      = 0; // 11
    virtual void GetLocalViewAngles(qangle& ang) = 0; // 12
    virtual void SetLocalViewAngles(qangle& ang) = 0; // 13
};

export namespace fd::valve
{
    using ::move_data;
    using ::prediction;
} // namespace fd::valve
