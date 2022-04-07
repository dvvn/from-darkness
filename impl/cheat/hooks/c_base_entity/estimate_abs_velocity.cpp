module;

//#include <nstd/enum_tools.h>

#include <cheat/hooks/instance.h>

module cheat.hooks.c_base_entity.estimate_abs_velocity;
import cheat.csgo.modules;
import cheat.csgo.interfaces.C_BaseEntity;
import cheat.hooks.base;

using namespace cheat;
using namespace csgo;
using namespace hooks;

CHEAT_HOOK_INSTANCE(c_base_entity, estimate_abs_velocity);

static void* target( ) noexcept
{
	const nstd::mem::basic_address<void> vtable = csgo_modules::client.find_vtable<C_BaseEntity>( );
	const auto index = csgo_modules::client.find_signature<"FF 90 ? ? 00 00 F3 0F 10 4C 24 18">( ).plus(2).deref<1>( ).divide(4);
	return vtable.deref<1>( )[index.value];
}

struct replace
{
	void fn(Vector& vel) noexcept
	{
		CHEAT_HOOK_CALL_ORIGINAL_MEMBER(vel);

#if 0
		using namespace nstd::enum_operators;

		const auto ent = this->get_object_instance( );

		if (ent->m_iEFlags( ) & m_iEFlags_t::EFL_DIRTY_ABSVELOCITY)
		{
			static auto calc_absolute_velocity = csgo_modules::client.find_signature<"55 8B EC 83 E4 F8 83 EC 1C 53 56 57 8B F9 F7">( ).cast<void (C_BaseEntity::*)()>( );
			dhooks::call_function(calc_absolute_velocity, ent);
		}

		vel = ent->m_vecAbsVelocity( );

		this->store_return_value(true);
#endif
	}
};

CHEAT_HOOK_INIT(c_base_entity, estimate_abs_velocity);