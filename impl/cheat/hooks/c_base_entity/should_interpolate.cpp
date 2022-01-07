#if 0

#include "should_interpolate.h"

using namespace cheat;
using namespace hooks::c_base_entity;

CHEAT_SERVICE_REGISTER_GAME(should_interpolate);

should_interpolate::should_interpolate( )
{
	this->wait_for_service<netvars>( );
}

nstd::address should_interpolate::get_target_method_impl( ) const
{
	const nstd::address vtable = csgo_modules::client.find_vtable<C_BaseEntity>( );
	const auto index = csgo_modules::client.find_signature<"8B 06 8B CE 8B 80 ? ? 00 00 FF D0 84 C0 74 5C">( ).add(6).deref(1).divide(4).value( );
	return vtable.ref<ptrdiff_t*>( )[index];
}

basic_service::load_result should_interpolate::load_impl( ) noexcept
{
	CHEAT_LOAD_HOOK_PROXY;
}

void should_interpolate::callback( )
{
#if /*!CHEAT_MODE_INGAME*/!FALSE
	CHEAT_CALL_BLOCKER
#else
	auto ent          = this->object_instance;
	auto client_class = ent->GetClientClass( );

	if (client_class->ClassID != ClassId::CCSPlayer)
		return;

	this->return_value_.store_value(false);
#endif
}
#endif