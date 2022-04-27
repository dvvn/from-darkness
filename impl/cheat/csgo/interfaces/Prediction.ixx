module;

#include <cstdint>

export module cheat.csgo.interfaces.Prediction;
export import cheat.csgo.interfaces.BaseHandle;
export import cheat.math.vector3;
export import cheat.math.qangle;
export import cheat.csgo.tools.UtlVector;

export namespace cheat::csgo
{
	class CMoveData
	{
	public:
		bool m_bFirstRunOfFunctions : 1;
		bool m_bGameCodeMovedPlayer : 1;
		int         m_nPlayerHandle;    // edict index on server, client entity handle on client=
		int         m_nImpulseCommand;  // Impulse command issued.
		math::vector3 m_vecViewAngles;    // Command view angles (local space)
		math::vector3 m_vecAbsViewAngles; // Command view angles (world space)
		int         m_nButtons;         // Attack buttons.
		int         m_nOldButtons;      // From host_client->oldbuttons;
		float       m_flForwardMove;
		float       m_flSideMove;
		float       m_flUpMove;
		float       m_flMaxSpeed;
		float       m_flClientMaxSpeed;
		math::vector3 m_vecVelocity; // edict::velocity        // Current movement direction.
		math::vector3 m_vecAngles;   // edict::angles
		math::vector3 m_vecOldAngles;
		float       m_outStepHeight; // how much you climbed this move
		math::vector3 m_outWishVel;    // This is where you tried 
		math::vector3 m_outJumpVel;    // This is your jump velocity
		math::vector3 m_vecConstraintCenter;
		float       m_flConstraintRadius;
		float       m_flConstraintWidth;
		float       m_flConstraintSpeedFactor;
		float       m_flUnknown[5];
		math::vector3 m_vecAbsOrigin;
	};

	class C_BasePlayer;

	class IPrediction
	{
		//std::byte pad0[0x4]; // 0x0000
	public:
		virtual ~IPrediction( ) = default;

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
		virtual void GetViewOrigin(math::vector3& org) = 0;      //8
		virtual void SetViewOrigin(math::vector3& org) = 0;      //9
		virtual void GetViewAngles(math::qangle& ang) = 0;      //10
		virtual void SetViewAngles(math::qangle& ang) = 0;      //11
		virtual void GetLocalViewAngles(math::qangle& ang) = 0; //12
		virtual void SetLocalViewAngles(math::qangle& ang) = 0; //13
	};
}
