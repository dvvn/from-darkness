module;
#include "root_includes.h"

#include <dhooks/includes.h>
#include <windows.h>

module cheat.service:root;
//import cheat.console;
import cheat.csgo.awaiter;
import dhooks;

using namespace cheat;
using dhooks::hook_holder_data;

services_loader::services_loader( ) = default;
services_loader::~services_loader( ) = default;

struct all_hooks_storage : services_loader::lazy_reset, std::vector<std::shared_ptr<hook_holder_data>>
{
};

struct unload_helper_data
{
	DWORD sleep;
	BOOL retval;
};

static DWORD WINAPI _Unload_helper(LPVOID data_packed)
{
	const auto data_ptr = static_cast<unload_helper_data*>(data_packed);
	const auto [sleep, retval] = *data_ptr;
	delete data_ptr;

	auto& loader = services_loader::get( );
	if (loader.load_thread.joinable( ))
	{
		loader.load_thread.request_stop( );
		loader.load_thread.join( );
	}
	auto handle = static_cast<HMODULE>(loader.module_handle);
	//destroy all except hooks
	auto all_hooks = loader.reset(true);
	dhooks::current_context::reset( );
	Sleep(sleep / 2);
	all_hooks.reset( ); //unhook force
	Sleep(sleep / 2);
	//we must close all threads before unload!
	FreeLibraryAndExitThread(handle, retval);
}

void services_loader::unload( )
{
	const auto data = new unload_helper_data{1500,TRUE};
	CreateThread(nullptr, 0, _Unload_helper, data, 0, nullptr);
}

auto services_loader::reset(bool deps_only)->reset_object
{
	this->set_state(service_state::unset);

	auto hooks = std::make_unique<all_hooks_storage>( );
	for (auto& d : this->_Deps<false>( ))
	{
		auto ptr = std::dynamic_pointer_cast<hook_holder_data>(std::move(d));
		if (!ptr)
			continue;
		ptr->unhook_after_call( );
		hooks->push_back(std::move(ptr));
	}

	if (deps_only)
	{
		for (auto& d : _Deps<false>( ))
		{
			value_type empty;
			std::swap(empty, d);
		}
	}
	else
	{
		services_loader tmp;
		std::swap(tmp, *this);
	}

	return hooks;
}

void services_loader::construct( ) noexcept
{
	//this->deps( ).get<console>( ).log("Cheat started");
	using namespace dhooks;
	//current_context::set(std::make_shared<context_safe>(std::make_unique<context>( )));
	current_context::set(std::make_shared<context>( ));
}

bool services_loader::load( ) noexcept
{
	//this->deps( ).get<console>( ).log("Cheat fully loaded");
	return true;
}



