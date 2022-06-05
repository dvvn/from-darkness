
export module fds.csgo.interfaces.MoveHelper;

export namespace fds::csgo
{
    class IClientEntity;

    class IMoveHelper
    {
      public:
        virtual void _vpad()                      = 0;
        virtual void SetHost(IClientEntity* host) = 0;
    };
} // namespace fds::csgo
