module;
#include "includes.h"

#include <dhooks/includes.h>

#include <cppcoro/sync_wait.hpp>

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

static constexpr auto _Load_async = []<class S, typename T >(S * service, T && executor, HMODULE module_handle)
{
	if (cppcoro::sync_wait(service->load(*executor)))
		return;
	auto delayed = service->reset( );
	using namespace std::chrono_literals;
	std::this_thread::sleep_for(200ms);
	::FreeLibrary(module_handle);
};

void services_loader::load_async(const std::shared_ptr<executor>&ex)
{
	load_thread = std::jthread(_Load_async, this, ex, module_handle);
}

void services_loader::load_async(std::unique_ptr<executor> && ex)
{
	load_thread = std::jthread(_Load_async, this, std::move(ex), module_handle);
}

bool services_loader::load_sync( )
{
	executor ex;
	return cppcoro::sync_wait(this->load(ex));
}

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
	loader.load_thread.request_stop( );
	auto handle = loader.module_handle;
	//destroy all except hooks
	auto all_hooks = loader.reset( );
	Sleep(sleep / 2);
	all_hooks.reset( ); //unhook force
	Sleep(sleep / 2);
	//we must close all threads before unload!
	FreeLibraryAndExitThread(loader.module_handle, retval);
}

void services_loader::unload( )
{
	const auto data = new unload_helper_data{1500,TRUE};
	CreateThread(nullptr, 0, _Unload_helper, data, 0, nullptr);
}

auto services_loader::reset( )->reset_object
{
	using stored_element = all_hooks_storage::value_type;
	auto hooks = std::make_unique<all_hooks_storage>( );

	for (auto& d : this->_Deps<false>( ))
	{
		auto ptr = std::dynamic_pointer_cast<hook_holder_data>(std::move(d));
		if (!ptr)
			continue;
		ptr->unhook_after_call( );
		hooks->push_back(std::move(ptr));
	}

	services_loader tmp;
	tmp = std::move(*this);

	return hooks;
}

void services_loader::load_async( ) noexcept
{
	//this->deps( ).get<console>( ).log("Cheat started");
	using namespace dhooks;
	//current_context::set(std::make_shared<context_safe>(std::make_unique<context>( )));
	current_context::set(std::make_shared<context>( ));
}
bool services_loader::load_impl( ) noexcept
{
	//this->deps( ).get<console>( ).log("Cheat fully loaded");
	return true;
}



