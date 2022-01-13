module;
#include "includes.h"

#include <dhooks/includes.h>

#include <cppcoro/sync_wait.hpp>

module cheat.service:root;
//import cheat.console;
import cheat.csgo.awaiter;
import dhooks;

using namespace cheat;

services_loader::services_loader( ) = default;
services_loader::~services_loader( ) = default;

struct all_hooks_storage : services_loader::lazy_reset
{
	using value_type = std::shared_ptr<dhooks::hook_holder_data>;
	using storage_type = std::vector<value_type>;

	storage_type vec;

	all_hooks_storage(storage_type&& storage) :vec(std::move(storage)) { }
};

void services_loader::load_async(const std::shared_ptr<executor>&ex)
{
	load_thread = std::jthread([=]
							   {
								   if (sync_wait(this->load(*ex)))
									   return;
								   auto hooks = this->reset( );
								   using namespace std::chrono_literals;
								   std::this_thread::sleep_for(200ms);
								   FreeLibrary(module_handle);
							   });
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

	auto handle = services_loader::get( ).module_handle;
	//destroy all except hooks
	auto all_hooks = services_loader::get( ).reset( );
	for (auto& h : static_cast<all_hooks_storage*>(all_hooks.get( ))->vec)
		h->unhook_after_call( );
	Sleep(sleep / 2);
	all_hooks.reset( ); //unhook force
	Sleep(sleep / 2);
	//we must close all threads before unload!
	FreeLibraryAndExitThread(handle, retval);
}

void services_loader::unload( )
{
	load_thread.request_stop( );
	const auto data = new unload_helper_data{1500,TRUE};
	CreateThread(nullptr, 0, _Unload_helper, data, 0, nullptr);
}

auto services_loader::reset( )->reset_object
{
	using stored_value = all_hooks_storage::value_type;
	using stored_element = stored_value::element_type;
	all_hooks_storage::storage_type hooks;

	for (value_type& d : this->_Deps<false>( ))
	{
		auto ptr = std::dynamic_pointer_cast<stored_element>(std::move(d));
		if (!ptr)
			continue;
		hooks.push_back(std::move(ptr));
	}

	services_loader tmp;
	tmp = std::move(*this);

	return std::make_unique<all_hooks_storage>(std::move(hooks));
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



