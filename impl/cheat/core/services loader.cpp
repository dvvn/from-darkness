#include "services loader.h"
#include "console.h"
#ifndef CHEAT_GUI_TEST
#include "csgo_awaiter.h"
#endif

#include <nstd/os/module info.h>
#include <nstd/os/threads.h>

#include <dhooks/hook_utils.h>

#include <robin_hood.h>

#include <cppcoro/sync_wait.hpp>

#include <Windows.h>

#include <functional>

using namespace cheat;
using namespace detail;

#ifndef CHEAT_GUI_TEST

struct unload_helper_data
{
	DWORD sleep;
	HMODULE handle;
	BOOL retval;
};

static DWORD WINAPI _Unload_helper(LPVOID data_packed)
{
	const auto data_ptr                = static_cast<unload_helper_data*>(data_packed);
	const auto [sleep, handle, retval] = *data_ptr;
	delete data_ptr;

	using dhooks::hook_holder_base;

	using hooks_storage = robin_hood::unordered_set<std::shared_ptr<hook_holder_base>>;
	using get_all_hooks_fn = std::function<void(const service_base& base, hooks_storage& set)>;
	const get_all_hooks_fn get_all_hooks = [&](const service_base& base, hooks_storage& set)
	{
		for (auto& s: base.services( ))
		{
			auto ptr = std::dynamic_pointer_cast<hook_holder_base>(s);
			if (ptr != nullptr)
				set.emplace(std::move(ptr));

			get_all_hooks(*s, set);
		}
	};

	auto loader = services_loader::get_ptr( );

	auto all_hooks = hooks_storage( );
	get_all_hooks(*loader, all_hooks);

	auto frozen = nstd::os::frozen_threads_storage(true);
	for (auto& h: all_hooks)
		h->disable( );
	frozen.clear( );

	loader->unload( ); //destroy all except hooks
	Sleep(sleep / 2);
	all_hooks.clear( ); //unhook all
	Sleep(sleep / 2);
	//we must be close all threads before unload!
	FreeLibraryAndExitThread(handle, retval);
}

HMODULE services_loader::my_handle( ) const
{
	return my_handle__;
}

void services_loader::load(HMODULE handle)
{
	my_handle__  = handle;
	load_thread_ = std::jthread([this]
	{
		auto ex = make_executor( );

		const auto load_helper = [&]
		{
			return sync_wait(this->load(ex));
		};

		if (!load_helper( ))
			this->unload( );
		else
			this->remove_service<csgo_awaiter>( );
	});
}

void services_loader::unload_delayed( )
{
	this->load_thread_.request_stop( );
	(void)this;
	const auto data = new unload_helper_data;
	data->sleep     = 1000;
	data->handle    = my_handle__;
	data->retval    = TRUE;

	CreateThread(nullptr, 0, _Unload_helper, data, 0, nullptr);
}

std::stop_token services_loader::load_thread_stop_token( ) const
{
	return load_thread_.get_stop_token( );
}
#endif

service_base::executor services_loader::make_executor( )
{
	return executor(/*std::min<size_t>(8, std::thread::hardware_concurrency( ))*/);
}

service_base::load_result services_loader::load_impl( ) noexcept
{
	CHEAT_CONSOLE_LOG("Cheat fully loaded");
	co_return (true);
}

services_loader::~services_loader( ) = default;
services_loader::services_loader( )  = default;

//---

void service_base::unload( )
{
	const auto loader = services_loader::get_ptr( );
	if (this->type( ) == loader->type( ))
		deps_.clear( );
	else
		loader->remove_service(this->type( ));
}
