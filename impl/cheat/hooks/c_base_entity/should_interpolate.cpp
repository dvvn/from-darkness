#include "should_interpolate.h"

using namespace cheat;
using namespace hooks::c_base_entity;

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
