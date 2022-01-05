module;

#include <cstdint>

export module cheat.csgo.interfaces:ClientEntity;
export import :ClientUnknown;
export import :ClientRenderable;
export import :ClientNetworkable;
export import :ClientThinkable;

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
		virtual bool GetSoundSpatialization(struct SpatializationInfo_t& info) = 0;
		virtual bool IsBlurred( ) = 0;
	};
}
