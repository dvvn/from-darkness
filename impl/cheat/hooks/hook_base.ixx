module;

#include <dhooks/wrapper.h>

export module cheat.hooks.base;
export import cheat.core.service;
export import cheat.core.csgo_interfaces;
export import cheat.core.services_loader;
export import cheat.core.console;

export namespace cheat::hooks
{
	template<typename T, typename Fn>
	struct hook_base : dynamic_service<T>, dhooks::select_hook_holder<Fn>
	{
	protected:
		//fix for broken compiler
		//most be renamed to load_impl .... override later
		basic_service::load_result load_impl2( ) noexcept /*override*/
		{
			if (!this->hook( ))
				co_return console::get( ).on_service_loaded<false>(this, "(Unable to setup hook)");
			if (!this->enable( ))
				co_return console::get( ).on_service_loaded<false>(this, "(Unable to enable hook)");

			co_return console::get( ).on_service_loaded(this);
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
