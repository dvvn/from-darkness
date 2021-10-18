#include "should interpolate.h"

#include "cheat/core/console.h"
#include "cheat/core/services loader.h"
#include "cheat/core/csgo modules.h"

#include "cheat/sdk/ClientClass.hpp"
#include "cheat/sdk/entity/C_BaseEntity.h"

// ReSharper disable once CppUnusedIncludeDirective
#include "cheat/netvars/config.h"
#include "cheat/netvars/netvars.h"

using namespace cheat;
using namespace hooks::c_base_entity;
using namespace csgo;

#ifndef CHEAT_GUI_TEST
using namespace nstd::address_pipe;
#endif

//utils::find_signature\("([a-z]+).*,.("[A-F0-9 ]+")
//find_signature\("([a-z0-9]+).*,.(".*")
//csgo_modules::$1.find_signature<$2>(
//
//utils::vtable_pointer<(.*)>\("([a-z0-9]+).*\)
//csgo_modules::$2.find_vtable<$1>()

should_interpolate::should_interpolate()
{
	this->wait_for_service<netvars>( );
}

CHEAT_HOOK_PROXY_INIT_FN(should_interpolate, FALSE)
CHEAT_HOOK_PROXY_TARGET_FN(should_interpolate, FALSE,
						   CHEAT_FIND_VTABLE(client,C_BaseEntity),
						   CHEAT_FIND_SIG(client,"8B 06 8B CE 8B 80 ? ? 00 00 FF D0 84 C0 74 5C",add(6),deref(1),divide(4),value));

void should_interpolate::callback()
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

CHEAT_REGISTER_SERVICE(should_interpolate);
