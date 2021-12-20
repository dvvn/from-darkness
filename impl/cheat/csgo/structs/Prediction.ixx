module;

#include <cstdint>

export module cheat.csgo.structs.Prediction;
import cheat.csgo.math.Vector;
import cheat.csgo.tools.UtlVector;
import cheat.csgo.structs.BaseHandle;

export namespace cheat::csgo
{
	class QAngle;

	class CMoveData
	{
	public:
		bool m_bFirstRunOfFunctions : 1;
		bool m_bGameCodeMovedPlayer : 1;
		int         m_nPlayerHandle;    // edict index on server, client entity handle on client=
		int         m_nImpulseCommand;  // Impulse command issued.
		Vector m_vecViewAngles;    // Command view angles (local space)
		Vector m_vecAbsViewAngles; // Command view angles (world space)
		int         m_nButtons;         // Attack buttons.
		int         m_nOldButtons;      // From host_client->oldbuttons;
		float       m_flForwardMove;
		float       m_flSideMove;
		float       m_flUpMove;
		float       m_flMaxSpeed;
		float       m_flClientMaxSpeed;
		Vector m_vecVelocity; // edict::velocity        // Current movement direction.
		Vector m_vecAngles;   // edict::angles
		Vector m_vecOldAngles;
		float       m_outStepHeight; // how much you climbed this move
		Vector m_outWishVel;    // This is where you tried 
		Vector m_outJumpVel;    // This is your jump velocity
		Vector m_vecConstraintCenter;
		float       m_flConstraintRadius;
		float       m_flConstraintWidth;
		float       m_flConstraintSpeedFactor;
		float       m_flUnknown[5];
		Vector m_vecAbsOrigin;
	};

	class C_BasePlayer;

	class IGameMovement
	{
	public:
		virtual ~IGameMovement( ) = 0;

		virtual void               ProcessMovement(C_BasePlayer* pPlayer, CMoveData* pMove) = 0;
		virtual void               Reset( ) = 0;
		virtual void               StartTrackPredictionErrors(C_BasePlayer* pPlayer) = 0;
		virtual void               FinishTrackPredictionErrors(C_BasePlayer* pPlayer) = 0;
		virtual void               DiffPrint(const char* fmt, ...) = 0;
		virtual const Vector& GetPlayerMins(bool ducked) const = 0;
		virtual const Vector& GetPlayerMaxs(bool ducked) const = 0;
		virtual const Vector& GetPlayerViewOffset(bool ducked) const = 0;
		virtual bool               IsMovingPlayerStuck( ) const = 0;
		virtual C_BasePlayer* GetMovingPlayer( ) const = 0;
		virtual void               UnblockPusher(C_BasePlayer* pPlayer, C_BasePlayer* pPusher) = 0;
		virtual void               SetupMovementBounds(CMoveData* pMove) = 0;
	};

	class CGameMovement : public IGameMovement
	{
	public:
		~CGameMovement( ) override = 0;
	};

	class IPrediction
	{
		//std::byte pad0[0x4]; // 0x0000
	public:
		virtual ~IPrediction( ) = 0;

		std::uintptr_t hLastGround;                // 0x0004
		bool           bInPrediction;              // 0x0008
		bool           bIsFirstTimePredicted;      // 0x0009
		bool           bEnginePaused;              // 0x000A
		bool           bOldCLPredictValue;         // 0x000B
		int            iPreviousStartFrame;        // 0x000C
		int            nIncomingPacketNumber;      // 0x0010
		float          flLastServerWorldTimeStamp; // 0x0014

		struct
		{
			bool                    bIsFirstTimePredicted;                    // 0x0018
			uint8_t               pad0[0x3];                                // 0x0019
			int                     nCommandsPredicted;                       // 0x001C
			int                     nServerCommandsAcknowledged;              // 0x0020
			int                     iPreviousAckHadErrors;                    // 0x0024
			float                   flIdealPitch;                             // 0x0028
			int                     iLastCommandAcknowledged;                 // 0x002C
			bool                    bPreviousAckErrorTriggersFullLatchReset;  // 0x0030
			CUtlVector<CBaseHandle> vecEntitiesWithPredictionErrorsInLastAck; // 0x0031
			bool                    bPerformedTickShift;                      // 0x0045
			//-
		} Split[1]; // 0x0018

#if 0
		bool InPrediction( )
		{
			typedef bool(__thiscall* oInPrediction)(void*);
			return CallVFunction<oInPrediction>(this, 14)(this);
		}

		void RunCommand(C_BasePlayer* player, CUserCmd* ucmd, IMoveHelper* moveHelper)
		{
			typedef void(__thiscall* oRunCommand)(void*, C_BasePlayer*, CUserCmd*, IMoveHelper*);
			return CallVFunction<oRunCommand>(this, 19)(this, player, ucmd, moveHelper);
		}

		void SetupMove(C_BasePlayer* player, CUserCmd* ucmd, IMoveHelper* moveHelper, void* pMoveData)
		{
			typedef void(__thiscall* oSetupMove)(void*, C_BasePlayer*, CUserCmd*, IMoveHelper*, void*);
			return CallVFunction<oSetupMove>(this, 20)(this, player, ucmd, moveHelper, pMoveData);
		}

		void FinishMove(C_BasePlayer* player, CUserCmd* ucmd, void* pMoveData)
		{
			typedef void(__thiscall* oFinishMove)(void*, C_BasePlayer*, CUserCmd*, void*);
			return CallVFunction<oFinishMove>(this, 21)(this, player, ucmd, pMoveData);
		}
#endif

		virtual void Init(void) = 0;     //1
		virtual void Shutdown(void) = 0; //2

		// Run prediction
		virtual void Update(
			int  startframe,            // World update ( un-modded ) most recently received
			bool validframe,            // Is frame data valid
			int  incoming_acknowledged, // Last command acknowledged to have been run by server (un-modded)
			int  outgoing_command       // Last command (most recent) sent to server (un-modded)
		) = 0;                      //3

// We are about to get a network update from the server.  We know the update #, so we can pull any
//  data purely predicted on the client side and transfer it to the new from data state.
		virtual void PreEntityPacketReceived(int commands_acknowledged, int current_world_update_packet, int server_ticks_elapsed) = 0; //4
		virtual void PostEntityPacketReceived(void) = 0;                                                                                //5
		virtual void PostNetworkDataReceived(int commands_acknowledged) = 0;                                                            //6

		virtual void OnReceivedUncompressedPacket(void) = 0; //7

		// The engine needs to be able to access a few predicted values
		virtual void GetViewOrigin(Vector& org) = 0;      //8
		virtual void SetViewOrigin(Vector& org) = 0;      //9
		virtual void GetViewAngles(QAngle& ang) = 0;      //10
		virtual void SetViewAngles(QAngle& ang) = 0;      //11
		virtual void GetLocalViewAngles(QAngle& ang) = 0; //12
		virtual void SetLocalViewAngles(QAngle& ang) = 0; //13
	};
}
