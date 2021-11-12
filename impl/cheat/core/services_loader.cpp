#include "services_loader.h"
#include "console.h"
#ifndef CHEAT_GUI_TEST
#include "csgo_awaiter.h"
#endif
#include "cheat/service/data.h"

#include <nstd/module/info.h>
#include <dhooks/context.h>
#include <dhooks/wrapper_fwd.h>

#include <cppcoro/sync_wait.hpp>
#include <cppcoro/static_thread_pool.hpp>
#include <cppcoro/task.hpp>

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

	const auto loader = services_loader::get_ptr( );

	auto all_hooks = loader->get_hooks(true);
	for (const auto& h: all_hooks)
		h->disable_after_call( );

	loader->unload( ); //destroy all except hooks
	Sleep(sleep / 2);
	all_hooks.clear( ); //unhook all
	Sleep(sleep / 2);
	//we must be close all threads before unload!
	FreeLibraryAndExitThread(handle, retval);
}

struct dummy_service final : service<dummy_service>
{
protected:
	load_result load_impl( ) noexcept override
	{
		co_return true;
	}
};

services_loader::~services_loader( ) = default;

services_loader::services_loader( )
{
	this->add_dependency(std::make_shared<dummy_service>( ));
}

HMODULE services_loader::my_handle( ) const
{
	return my_handle__;
}

void services_loader::load(HMODULE handle)
{
	//probably not best place for this
	using namespace dhooks;
	current_context::set(std::make_shared<context_safe>(std::make_unique<context>( )));

	my_handle__  = handle;
	load_thread_ = std::jthread([this]
	{
		const auto load_helper = [&]
		{
			const auto ex = this->get_executor( );
			return sync_wait(this->load(*ex));
		};

		if (!load_helper( ))
			this->unload( );
		else
			this->remove_service(csgo_awaiter::type( ));
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

auto services_loader::get_executor(size_t threads_count) -> std::shared_ptr<executor>
{
	if (!executor_.expired( ))
		return executor_.lock( );

	auto out  = std::make_shared<executor>(threads_count);
	executor_ = out;
	return out;
}

using dhooks::hook_holder_base;

// ReSharper disable once CppMemberFunctionMayBeConst
std::vector<stored_service<hook_holder_base>> services_loader::get_hooks(bool steal)
{
	std::vector<stored_service<hook_holder_base>> out;

	auto& deps = *this->deps_;
	for (auto& d: deps)
	{
		auto ptr = std::dynamic_pointer_cast<hook_holder_base>(d);
		if (!ptr)
			continue;

		out.push_back({std::move(ptr), stored_service_cast_tag{}});

		if (steal)
		{
			value_type empty;
			std::swap(empty, d);
		}
	}

	if (steal && !out.empty( ))
	{
		auto to_remove = std::ranges::remove(deps, value_type( ));
		deps.erase(to_remove.begin( ), to_remove.end( ));
	}

	return out;
}

basic_service::load_result services_loader::load_impl( ) noexcept
{
	CHEAT_CONSOLE_LOG("Cheat fully loaded");
	co_return true;
}

//---

// ReSharper disable once CppMemberFunctionMayBeConst
void basic_service::unload( )
{
	auto loader = services_loader::get_ptr( );
	if (this->type( ) == loader->type( ))
		reload_one_instance(*loader);
	else
		loader->remove_service(this->type( ));
}

// ReSharper disable once CppMemberFunctionMayBeConst
void basic_service::remove_service(const std::type_info& info)
{
	const auto loader = services_loader::get_ptr( );
	if (this->type( ) == loader->type( ))
	{
		auto& deps       = *loader->deps_;
		const auto found = loader->find_service(info);
		deps.erase(deps.begin( ) + std::distance(deps._Unchecked_begin( ), found));
	}
	else
	{
		auto dummy                = loader->find_service(typeid(dummy_service));
		*this->find_service(info) = std::move(*dummy);
	}
}

//-----

void basic_service_shared::add_to_loader(value_type&& srv) const
{
	const auto loader = services_loader::get_ptr( );
	loader->add_dependency(std::move(srv));
}

// ReSharper disable once CppMemberFunctionMayBeStatic
auto basic_service_shared::get_from_loader(const std::type_info& info) const -> value_type*
{
	const auto loader = services_loader::get_ptr( );
	return loader->find_service(info);
}
