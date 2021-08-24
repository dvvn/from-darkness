#include "estimate abs velocity.h"

using namespace cheat;
using namespace hooks;
using namespace c_base_entity;

using namespace csgo;

nstd::address estimate_abs_velocity::get_target_method_impl( ) const
{
	const auto vtable = utils::vtable_pointer<C_BaseEntity>("client.dll");
	const auto index  = utils::find_signature("client.dll", "FF 90 ? ? 00 00 F3 0F 10 4C 24 18").add(2).deref(1).divide(4).value( );

	return dhooks::_Pointer_to_virtual_class_table(vtable)[index];
}

void estimate_abs_velocity::callback(Vector& vel)
{
	const auto ent = this->object_instance;
	if (m_iEFlags_t(ent->m_iEFlags( )).has(m_iEFlags_t::EFL_DIRTY_ABSVELOCITY))
	{
		// ReSharper disable once CppInconsistentNaming
		static auto CalcAbsoluteVelocity_fn = []
		{
			const auto           addr = utils::find_signature("client.dll", "55 8B EC 83 E4 F8 83 EC 1C 53 56 57 8B F9 F7");
			void (C_BaseEntity::*fn)( );
			reinterpret_cast<void*&>(fn) = addr.ptr<void>( );
			return fn;
		}( );

		dhooks::_Call_function(CalcAbsoluteVelocity_fn, ent);
	}

	vel = ent->m_vecAbsVelocity( );

	this->return_value_.set_original_called(true);
}
