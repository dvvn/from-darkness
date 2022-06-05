module;

#include <cstdint>

export module fds.csgo.interfaces.ClientEntity;
export import fds.csgo.interfaces.ClientUnknown;
export import fds.csgo.interfaces.ClientRenderable;
export import fds.csgo.interfaces.ClientNetworkable;
export import fds.csgo.interfaces.ClientThinkable;
export import fds.math.vector3;
export import fds.math.qangle;

export namespace fds::csgo
{
    struct SpatializationInfo_t;

    class IClientEntity : public IClientUnknown, public IClientRenderable, public IClientNetworkable, public IClientThinkable
    {
      public:
        virtual const math::vector3& GetAbsOrigin() const                      = 0;
        virtual const math::qangle& GetAbsAngles() const                       = 0;
        virtual void* GetMouth()                                               = 0;
        virtual bool GetSoundSpatialization(struct SpatializationInfo_t& info) = 0;
        virtual bool IsBlurred()                                               = 0;
    };
} // namespace fds::csgo
