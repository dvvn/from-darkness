#pragma once
#include "UtlVector.hpp"

namespace cheat::csgo
{
	class CBaseHandle;
	class CMoveData
	{
	public:
		bool m_bFirstRunOfFunctions : 1;
		bool m_bGameCodeMovedPlayer : 1;
		int         m_nPlayerHandle;    // edict index on server, client entity handle on client=
		int         m_nImpulseCommand;  // Impulse command issued.
		utl::Vector m_vecViewAngles;    // Command view angles (local space)
		utl::Vector m_vecAbsViewAngles; // Command view angles (world space)
		int         m_nButtons;         // Attack buttons.
		int         m_nOldButtons;      // From host_client->oldbuttons;
		float       m_flForwardMove;
		float       m_flSideMove;
		float       m_flUpMove;
		float       m_flMaxSpeed;
		float       m_flClientMaxSpeed;
		utl::Vector m_vecVelocity; // edict::velocity        // Current movement direction.
		utl::Vector m_vecAngles;   // edict::angles
		utl::Vector m_vecOldAngles;
		float       m_outStepHeight; // how much you climbed this move
		utl::Vector m_outWishVel;    // This is where you tried 
		utl::Vector m_outJumpVel;    // This is your jump velocity
		utl::Vector m_vecConstraintCenter;
		float       m_flConstraintRadius;
		float       m_flConstraintWidth;
		float       m_flConstraintSpeedFactor;
		float       m_flUnknown[5];
		utl::Vector m_vecAbsOrigin;
	};

	class C_BasePlayer;

	class IGameMovement
	{
	public:
		virtual ~IGameMovement( ) = 0;

		virtual auto ProcessMovement(C_BasePlayer* pPlayer, CMoveData* pMove) -> void = 0;
		virtual auto Reset( ) -> void = 0;
		virtual auto StartTrackPredictionErrors(C_BasePlayer* pPlayer) -> void = 0;
		virtual auto FinishTrackPredictionErrors(C_BasePlayer* pPlayer) -> void = 0;
		virtual auto DiffPrint(const char* fmt, ...) -> void = 0;
		virtual auto GetPlayerMins(bool ducked) const -> const utl::Vector& = 0;
		virtual auto GetPlayerMaxs(bool ducked) const -> const utl::Vector& = 0;
		virtual auto GetPlayerViewOffset(bool ducked) const -> const utl::Vector& = 0;
		virtual auto IsMovingPlayerStuck( ) const -> bool = 0;
		virtual auto GetMovingPlayer( ) const -> C_BasePlayer* = 0;
		virtual auto UnblockPusher(C_BasePlayer* pPlayer, C_BasePlayer* pPusher) -> void = 0;
		virtual auto SetupMovementBounds(CMoveData* pMove) -> void = 0;
	};

	class CGameMovement: public IGameMovement
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
			std::byte               pad0[0x3];                                // 0x0019
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

		virtual auto Init(void) -> void = 0;//1
		virtual auto Shutdown(void) -> void = 0;//2

		// Run prediction
		virtual auto Update(
				int  startframe,            // World update ( un-modded ) most recently received
				bool validframe,            // Is frame data valid
				int  incoming_acknowledged, // Last command acknowledged to have been run by server (un-modded)
				int  outgoing_command       // Last command (most recent) sent to server (un-modded)
				)
		-> void = 0;//3

		// We are about to get a network update from the server.  We know the update #, so we can pull any
		//  data purely predicted on the client side and transfer it to the new from data state.
		virtual auto PreEntityPacketReceived(int commands_acknowledged, int current_world_update_packet, int server_ticks_elapsed) -> void = 0;//4
		virtual auto PostEntityPacketReceived(void) -> void = 0;//5
		virtual auto PostNetworkDataReceived(int commands_acknowledged) -> void = 0;//6

		virtual auto OnReceivedUncompressedPacket(void) -> void = 0;//7

		// The engine needs to be able to access a few predicted values
		virtual auto GetViewOrigin(utl::Vector& org) -> void = 0;//8
		virtual auto SetViewOrigin(utl::Vector& org) -> void = 0;//9
		virtual auto GetViewAngles(utl::QAngle& ang) -> void = 0;//10
		virtual auto SetViewAngles(utl::QAngle& ang) -> void = 0;//11
		virtual auto GetLocalViewAngles(utl::QAngle& ang) -> void = 0;//12
		virtual auto SetLocalViewAngles(utl::QAngle& ang) -> void = 0;//13
	};
}
