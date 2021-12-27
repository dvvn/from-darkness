module;

#include <cstdint>

export module cheat.csgo.structs.ClientEntity;
import cheat.csgo.structs.ClientUnknown;
import cheat.csgo.structs.ClientRenderable;
import cheat.csgo.structs.ClientNetworkable;
import cheat.csgo.structs.ClientThinkable;

export namespace cheat::csgo
{
	struct SpatializationInfo_t;

	class Vector;
	class QAngle;

	class IClientEntity : public IClientUnknown, public IClientRenderable, public IClientNetworkable, public IClientThinkable
	{
	public:
		virtual const Vector& GetAbsOrigin( ) const = 0;
		virtual const QAngle& GetAbsAngles( ) const = 0;
		virtual void* GetMouth( ) = 0;
		virtual bool               GetSoundSpatialization(struct SpatializationInfo_t& info) = 0;
		virtual bool               IsBlurred( ) = 0;
	};
}
