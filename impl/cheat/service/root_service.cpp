module;
#include "root_includes.h"

#include <dhooks/includes.h>
#include <windows.h>

module cheat.service:root;
//import cheat.console;
import dhooks;
import nstd.rtlib;

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
	auto handle = static_cast<HMODULE>(loader.module_handle);
	//destroy all except hooks
	auto all_hooks = loader.reset(true);
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
	auto deps = this->_Deps<false>( );
	if (this->state( ) == service_state::unset)
	{
		runtime_assert(std::ranges::count(deps, nullptr) == deps.size( ));
		return nullptr;
	}

	this->set_state(service_state::unset);

	auto hooks = std::make_unique<all_hooks_storage>( );
	//todo: scal all deps not root only
	for (auto& d : deps)
	{
		auto ptr = std::dynamic_pointer_cast<hook_holder_data>(std::move(d));
		if (!ptr)
			continue;
		ptr->disable_after_call( );
		hooks->push_back(std::move(ptr));
	}

	if (deps_only)
	{
		for (auto& d : deps)
		{
			value_type empty;
			std::swap(empty, d);
		}
	}
	else
	{
		this->_Reload( );
	}

	return hooks;
}

void services_loader::construct( ) noexcept
{
	//this->deps( ).get<console>( ).log("Cheat started");
}

bool services_loader::load( ) noexcept
{
	//this->deps( ).get<console>( ).log("Cheat fully loaded");
	services_cache::_Reload( );
	return true;
}



