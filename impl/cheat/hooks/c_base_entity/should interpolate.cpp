#include "should interpolate.h"

#include "cheat/core/csgo interfaces.h"
#include "cheat/core/helpers.h"
#include "cheat/netvars/netvars.h"
#include "cheat/players/players list.h"
#include "cheat/sdk/ClientClass.hpp"
#include "cheat/sdk/GlobalVars.hpp"
#include "cheat/sdk/IClientEntityList.hpp"

using namespace cheat;
using namespace hooks;
using namespace c_base_entity;
using namespace utl;
using namespace csgo;

should_interpolate::should_interpolate( )
{
}

bool should_interpolate::Do_load( )
{
	using namespace address_pipe;

	this->target_func_ = method_info::make_member_virtual
			(
			 std::bind_front(_Vtable_pointer<C_BaseEntity>, "client.dll"/*, &csgo_interfaces::local_player*/),
			 std::bind_front(_Find_signature, "client.dll", "8B 06 8B CE 8B 80 ? ? 00 00 FF D0 84 C0 74 5C") | add(6) | deref(1) | divide(4) | value
			);

	this->hook( );
	this->enable( );

	return true;
}

void should_interpolate::Callback( )
{
	auto ent          = this->Target_instance( );
	auto client_class = ent->GetClientClass( );

	if (client_class->ClassID != ClassId::CCSPlayer)
		return;

	this->return_value_.store_value(false);
}
