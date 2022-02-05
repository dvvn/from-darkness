module;

#include "base_includes.h"

export module cheat.hooks.base;
export import cheat.service;
export import dhooks;

export namespace cheat::hooks
{
	//todo: template in hooks for return address

	template<typename T, typename Fn, size_t UniqueIdx = 0>
	struct hook_base : dynamic_service<T>, dhooks::select_hook_holder<Fn, UniqueIdx>
	{
	protected:
		bool load( ) noexcept override
		{
			return this->hook( ) && this->enable( );
		}
	};
}

//#pragma once
////#include "cheat/core/console.h"
//#include "cheat/core/csgo_interfaces.h"
//
//#include <dhooks/wrapper.h>
//
//#define CHEAT_LOAD_HOOK_PROXY \
//if(!this->hook( )) \
//	CHEAT_SERVICE_NOT_LOADED("Unable to setup hook"); \
//if(!this->enable()) \
//	CHEAT_SERVICE_NOT_LOADED("Unable to enable hook"); \
//CHEAT_SERVICE_LOADED
