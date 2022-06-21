export module fd.csgo.interfaces.GameMovement;
export import fd.csgo.interfaces.Prediction;

export namespace fd::csgo
{
    class IGameMovement
    {
      public:
        virtual ~IGameMovement() = default;

        virtual void ProcessMovement(C_BasePlayer* pPlayer, CMoveData* pMove)    = 0;
        virtual void Reset()                                                     = 0;
        virtual void StartTrackPredictionErrors(C_BasePlayer* pPlayer)           = 0;
        virtual void FinishTrackPredictionErrors(C_BasePlayer* pPlayer)          = 0;
        virtual void DiffPrint(const char* fmt, ...)                             = 0;
        virtual const math::vector3& GetPlayerMins(bool ducked) const            = 0;
        virtual const math::vector3& GetPlayerMaxs(bool ducked) const            = 0;
        virtual const math::vector3& GetPlayerViewOffset(bool ducked) const      = 0;
        virtual bool IsMovingPlayerStuck() const                                 = 0;
        virtual C_BasePlayer* GetMovingPlayer() const                            = 0;
        virtual void UnblockPusher(C_BasePlayer* pPlayer, C_BasePlayer* pPusher) = 0;
        virtual void SetupMovementBounds(CMoveData* pMove)                       = 0;
    };

    class CGameMovement : public IGameMovement
    {
    };
} // namespace fd::csgo
