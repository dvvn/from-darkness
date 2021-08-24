#include "should interpolate.h"

#include "cheat/sdk/ClientClass.hpp"

using namespace cheat;
using namespace hooks;
using namespace c_base_entity;
using namespace csgo;

nstd::address should_interpolate::get_target_method_impl( ) const
{
	const auto vtable = utils::vtable_pointer<C_BaseEntity>("client.dll");
	const auto index  = utils::find_signature("client.dll", "8B 06 8B CE 8B 80 ? ? 00 00 FF D0 84 C0 74 5C").add(6).deref(1).divide(4).value( );

	return dhooks::_Pointer_to_virtual_class_table(vtable)[index];
}

void should_interpolate::callback( )
{
	auto ent          = this->object_instance;
	auto client_class = ent->GetClientClass( );

	if (client_class->ClassID != ClassId::CCSPlayer)
		return;

	this->return_value_.store_value(false);
}
