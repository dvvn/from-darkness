module;

#include <cstdint>

export module fd.csgo.interfaces.ClientEntity;
export import fd.csgo.interfaces.ClientUnknown;
export import fd.csgo.interfaces.ClientRenderable;
export import fd.csgo.interfaces.ClientNetworkable;
export import fd.csgo.interfaces.ClientThinkable;
export import fd.math.vector3;
export import fd.math.qangle;

export namespace fd::csgo
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
} // namespace fd::csgo
