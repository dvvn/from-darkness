
export module fd.csgo.interfaces.MoveHelper;

export namespace fd::csgo
{
    class IClientEntity;

    class IMoveHelper
    {
      public:
        virtual void _vpad()                      = 0;
        virtual void SetHost(IClientEntity* host) = 0;
    };
} // namespace fd::csgo
