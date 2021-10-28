#pragma once
//#include "cheat/core/console.h"
#include "cheat/core/csgo_interfaces.h"

#include <dhooks/hook_utils.h>

#define CHEAT_LOAD_HOOK_PROXY \
if(!this->hook( )) \
	CHEAT_SERVICE_NOT_LOADED("Unable to setup hook"); \
if(!this->enable()) \
	CHEAT_SERVICE_NOT_LOADED("Unable to enable hook"); \
CHEAT_SERVICE_LOADED
