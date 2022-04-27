module;

#include <cstdint>

export module cheat.csgo.interfaces.ClientEntity;
export import cheat.csgo.interfaces.ClientUnknown;
export import cheat.csgo.interfaces.ClientRenderable;
export import cheat.csgo.interfaces.ClientNetworkable;
export import cheat.csgo.interfaces.ClientThinkable;
export import cheat.math.vector3;
export import cheat.math.qangle;

export namespace cheat::csgo
{
	struct SpatializationInfo_t;

	class IClientEntity : public IClientUnknown, public IClientRenderable, public IClientNetworkable, public IClientThinkable
	{
	public:
		virtual const math::vector3& GetAbsOrigin( ) const = 0;
		virtual const math::qangle& GetAbsAngles( ) const = 0;
		virtual void* GetMouth( ) = 0;
		virtual bool GetSoundSpatialization(struct SpatializationInfo_t& info) = 0;
		virtual bool IsBlurred( ) = 0;
	};
}
