module;

#include <string_view>

module cheat.hooks.c_base_entity.estimate_abs_velocity;
import cheat.csgo.modules;
import cheat.csgo.interfaces.C_BaseEntity;
import cheat.hooks.hook;

using namespace cheat;
using namespace csgo;
using namespace hooks;
using namespace c_base_entity;

struct estimate_abs_velocity_impl final : estimate_abs_velocity, hook, hook_instance_member<estimate_abs_velocity_impl>
{
	estimate_abs_velocity_impl( )
	{
		const auto vtable = csgo_modules::client.find_vtable<C_BaseEntity>( );
		const auto index = csgo_modules::client.find_signature<"FF 90 ? ? 00 00 F3 0F 10 4C 24 18">( ).plus(2).deref<1>( ).divide(4);

		entry_type entry;
		entry.set_target_method({vtable,index});
		entry.set_replace_method(&estimate_abs_velocity_impl::callback);

		this->init(std::move(entry));
	}

	void callback( ) const noexcept
	{
		call_original( );


#if 0
		using namespace nstd::enum_operators;

		const auto ent = this->get_object_instance( );

		if(ent->m_iEFlags( ) & m_iEFlags_t::EFL_DIRTY_ABSVELOCITY)
		{
			static auto calc_absolute_velocity = csgo_modules::client.find_signature<"55 8B EC 83 E4 F8 83 EC 1C 53 56 57 8B F9 F7">( ).cast<void (C_BaseEntity::*)()>( );
			dhooks::call_function(calc_absolute_velocity, ent);
		}

		vel = ent->m_vecAbsVelocity( );

		this->store_return_value(true);
#endif
	}
};

std::string_view estimate_abs_velocity::class_name( ) const noexcept
{
	return "hooks::c_base_animating";
}

std::string_view estimate_abs_velocity::function_name( ) const noexcept
{
	return "estimate_abs_velocity";
}

template<>
template<>
nstd::one_instance_getter<estimate_abs_velocity*>::one_instance_getter(const std::in_place_index_t<0>)
	:item_(estimate_abs_velocity_impl::get_ptr( ))
{
}