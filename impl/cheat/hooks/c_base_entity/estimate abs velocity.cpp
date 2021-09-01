#include "estimate abs velocity.h"

#include "cheat/core/csgo modules.h"
#include "cheat/sdk/Vector.hpp"
#include "cheat/sdk/entity/C_BaseEntity.h"

#include <nstd/enum overloads.h>

using namespace cheat;
using namespace hooks;
using namespace c_base_entity;

using namespace csgo;

estimate_abs_velocity::estimate_abs_velocity( )
	: service_sometimes_skipped(
#if defined(CHEAT_GUI_TEST) || defined(CHEAT_NETVARS_UPDATING)
								true
#else
								false
#endif
							   )
{
}

nstd::address estimate_abs_velocity::get_target_method_impl( ) const
{
	const auto vtable = csgo_modules::client.find_vtable<C_BaseEntity>( );
	const auto index  = csgo_modules::client.find_signature<"FF 90 ? ? 00 00 F3 0F 10 4C 24 18">( ).add(2).deref(1).divide(4).value( );

	return dhooks::_Pointer_to_virtual_class_table(vtable)[index];
}

void estimate_abs_velocity::callback(Vector& vel)
{
#if !__has_include("cheat/sdk/generated/C_BaseEntity_h")
#pragma message(__FUNCTION__ ": skipped")
#else
	const auto ent = this->object_instance;
	if (static_cast<m_iEFlags_t>(ent->m_iEFlags( )) & (m_iEFlags_t::EFL_DIRTY_ABSVELOCITY))
	{
		// ReSharper disable once CppInconsistentNaming
		static auto CalcAbsoluteVelocity_fn = []
		{
			const auto           addr = csgo_modules::client.find_signature<"55 8B EC 83 E4 F8 83 EC 1C 53 56 57 8B F9 F7">( );
			void (C_BaseEntity::*fn)( );
			reinterpret_cast<void*&>(fn) = addr.ptr<void>( );
			return fn;
		}( );

		dhooks::_Call_function(CalcAbsoluteVelocity_fn, ent);
	}

	vel = ent->m_vecAbsVelocity( );

	this->return_value_.set_original_called(true);
#endif
}
