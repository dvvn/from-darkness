module;

#include "base_includes.h"

export module cheat.hooks.base;
export import cheat.service;
export import dhooks;

export namespace cheat::hooks
{
	template<typename T, typename Fn>
	struct hook_base : dynamic_service<T>, dhooks::select_hook_holder<Fn>
	{
	protected:
		bool load_impl( ) noexcept override
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
