#include "cheat/core/csgo_interfaces.h"

import cheat.csgo.structs;

using namespace cheat;
using namespace csgo;

IHandleEntity* CBaseHandle::Get( ) const
{
	/*if (!IsValid( ))
		return 0;*/
	return cheat::csgo_interfaces::get( )->entity_list->GetClientEntityFromHandle(*this);
}
