#pragma once

#include <fd/valve/prediction.h>

namespace fd::valve
{
    class base_player;

    struct game_movement
    {
        virtual ~game_movement() = default;

        virtual void ProcessMovement(base_player* pPlayer, move_data* pMove)   = 0;
        virtual void Reset()                                                   = 0;
        virtual void StartTrackPredictionErrors(base_player* pPlayer)          = 0;
        virtual void FinishTrackPredictionErrors(base_player* pPlayer)         = 0;
        virtual void DiffPrint(const char* fmt, ...)                           = 0;
        virtual const vector3& GetPlayerMins(bool ducked) const                = 0;
        virtual const vector3& GetPlayerMaxs(bool ducked) const                = 0;
        virtual const vector3& GetPlayerViewOffset(bool ducked) const          = 0;
        virtual bool IsMovingPlayerStuck() const                               = 0;
        virtual base_player* GetMovingPlayer() const                           = 0;
        virtual void UnblockPusher(base_player* pPlayer, base_player* pPusher) = 0;
        virtual void SetupMovementBounds(move_data* pMove)                     = 0;
    };
} // namespace fd::valve
