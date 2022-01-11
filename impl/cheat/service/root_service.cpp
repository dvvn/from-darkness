module;
#include "includes.h"
#include <cppcoro/sync_wait.hpp>
#include <dhooks/includes.h>

module cheat.service:root;
//import cheat.console;
#ifndef CHEAT_GUI_TEST
import cheat.csgo.awaiter;
#endif
import dhooks;

using namespace cheat;

services_loader::services_loader( ) = default;
services_loader::~services_loader( ) = default;


#ifdef CHEAT_GUI_TEST

bool services_loader::load( )
{
	const auto executor = this->get_executor( );
	return cppcoro::sync_wait(basic_service::load(*executor));
}

#else

HMODULE services_loader::my_handle( ) const
{
	return own_handle_;
}

void services_loader::load(HMODULE handle)
{

	own_handle_ = handle;
	load_thread_ = std::jthread([this]
								{
									const auto ex = this->get_executor( );
									if (!sync_wait(this->load(*ex)))
										this->unload( );
								});
}

struct unload_helper_data
{
	DWORD sleep;
	HMODULE handle;
	BOOL retval;
};

static DWORD WINAPI _Unload_helper(LPVOID data_packed)
{
	const auto data_ptr = static_cast<unload_helper_data*>(data_packed);
	const auto [sleep, handle, retval] = *data_ptr;
	delete data_ptr;

	//destroy all except hooks
	auto all_hooks = services_loader::get( ).reset( );
	Sleep(sleep / 2);
	all_hooks.reset( ); //unhook all
	Sleep(sleep / 2);
	//we must close all threads before unload!
	FreeLibraryAndExitThread(handle, retval);
}

std::stop_token services_loader::load_thread_stop_token( ) const
{
	return load_thread_.get_stop_token( );
}
#endif

void services_loader::unload( )
{
#ifdef CHEAT_GUI_TEST
	this->reset( );
#else
	this->load_thread_.request_stop( );
	(void)this;
	const auto data = new unload_helper_data{1000,own_handle_,TRUE};
	CreateThread(nullptr, 0, _Unload_helper, data, 0, nullptr);
#endif
}

struct all_hooks_storage : services_loader::lazy_reset
{
	using value_type = std::shared_ptr<dhooks::hook_holder_data>;
	using storage_type = std::vector<value_type>;

	storage_type vec;

	all_hooks_storage(storage_type&& storage) :vec(std::move(storage)) { }
};

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
		ptr->disable_after_call( );
		hooks.push_back(std::move(ptr));
	}

	services_loader tmp;
	tmp = std::move(*this);

	return std::make_unique<all_hooks_storage>(std::move(hooks));
}

auto services_loader::get_executor(size_t threads_count) -> executor_shared
{
	if (!executor_.expired( ))
		return executor_.lock( );

	auto out = std::make_shared<executor>(threads_count);
	executor_ = out;
	return out;
}

void services_loader::load_async( ) noexcept
{
	//this->deps( ).get<console>( ).log("Cheat started");
	using namespace dhooks;
	current_context::set(std::make_shared<context_safe>(std::make_unique<context>( )));
}
bool services_loader::load_impl( ) noexcept
{
	//this->deps( ).get<console>( ).log("Cheat fully loaded");
	return true;
}



