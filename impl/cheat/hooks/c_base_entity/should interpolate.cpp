#include "should interpolate.h"

#include "cheat/sdk/ClientClass.hpp"

using namespace cheat;
using namespace hooks;
using namespace c_base_entity;
using namespace csgo;

//utils::find_signature\("([a-z]+).*,.("[A-F0-9 ]+")
//find_signature\("([a-z0-9]+).*,.(".*")
//csgo_modules::$1.find_signature<$2>(
//
//utils::vtable_pointer<(.*)>\("([a-z0-9]+).*\)
//csgo_modules::$2.find_vtable<$1>()

should_interpolate::should_interpolate( )
{
	this->add_service<netvars>( );
}

nstd::address should_interpolate::get_target_method_impl( ) const
{
	const auto vtable = csgo_modules::client.find_vtable<C_BaseEntity>( );
	const auto index  = csgo_modules::client.find_signature<"8B 06 8B CE 8B 80 ? ? 00 00 FF D0 84 C0 74 5C">( ).add(6).deref(1).divide(4).value( );

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
