#include "services_loader.h"
#include "console.h"

#ifndef CHEAT_GUI_TEST
#include "csgo_awaiter.h"
#endif

#include "cheat/service/stored.h"

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

struct all_hooks_storage::impl : std::vector<value_type>
{
};

all_hooks_storage::all_hooks_storage( )
{
	impl_ = std::make_unique<impl>( );
}

all_hooks_storage::~all_hooks_storage( )                                      = default;
all_hooks_storage::all_hooks_storage(all_hooks_storage&&) noexcept            = default;
all_hooks_storage& all_hooks_storage::operator=(all_hooks_storage&&) noexcept = default;

all_hooks_storage::value_type* all_hooks_storage::begin( ) const
{
	return impl_->_Unchecked_begin( );
}

all_hooks_storage::value_type* all_hooks_storage::end( ) const
{
	return impl_->_Unchecked_end( );
}

void all_hooks_storage::push_back(value_type&& val)
{
	impl_->push_back(std::move(val));
}

void all_hooks_storage::push_back(const value_type& val)
{
	impl_->push_back(val);
}

bool all_hooks_storage::empty( ) const
{
	return impl_->empty( );
}

size_t all_hooks_storage::size( ) const
{
	return impl_->size( );
}

void all_hooks_storage::clear( )
{
	auto impl2 = std::make_unique<impl>( );
	std::swap(impl_, impl2);
}

//---
struct services_loader::impl
{
	std::weak_ptr<executor> executor;

#ifndef CHEAT_GUI_TEST
	HMODULE own_handle = nullptr;
	std::jthread load_thread;
#endif
};

services_loader::services_loader( )
{
	impl_ = std::make_unique<impl>( );
}

services_loader::~services_loader( ) = default;

static void _Setup_hooks_context( )
{
	//probably not best place for this
	using namespace dhooks;
	current_context::set(std::make_shared<context_safe>(std::make_unique<context>( )));
}

#ifdef CHEAT_GUI_TEST

bool services_loader::load( )
{
	_Setup_hooks_context( );
	const auto executor = this->get_executor( );
	return sync_wait(basic_service::load(*executor));
}

#else

HMODULE services_loader::my_handle( ) const
{
	return impl_->own_handle;
}

void services_loader::load(HMODULE handle)
{
	_Setup_hooks_context( );

	impl_->own_handle  = handle;
	impl_->load_thread = std::jthread([this]
	{
		const auto load_helper = [&]
		{
			const auto ex = this->get_executor( );
			return sync_wait(this->load(*ex));
		};

		if (!load_helper( ))
			this->unload( );
		else
			this->erase(csgo_awaiter::type( ));
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

void services_loader::unload_delayed( )
{
	this->impl_->load_thread.request_stop( );
	(void)this;
	const auto data = new unload_helper_data;
	data->sleep     = 1000;
	data->handle    = impl_->own_handle;
	data->retval    = TRUE;

	CreateThread(nullptr, 0, _Unload_helper, data, 0, nullptr);
}

std::stop_token services_loader::load_thread_stop_token( ) const
{
	return impl_->load_thread.get_stop_token( );
}
#endif

// ReSharper disable once CppMemberFunctionMayBeConst
auto services_loader::get_executor(size_t threads_count) -> executor_shared
{
	auto& exc = impl_->executor;
	if (!exc.expired( ))
		return exc.lock( );

	auto out = std::make_shared<executor>(threads_count);
	exc      = out;
	return out;
}

auto services_loader::get_executor( ) -> executor_shared
{
	return get_executor(std::thread::hardware_concurrency( ));
}

// ReSharper disable once CppMemberFunctionMayBeConst
auto services_loader::get_hooks(bool steal) -> all_hooks_storage
{
	using stored_value = all_hooks_storage::value_type;
	using stored_element = stored_value::element_type;
	all_hooks_storage out;

	this->erase_all([steal, &out](const value_type& val)
	{
		auto ptr = steal
					   ? std::dynamic_pointer_cast<stored_element>(const_cast<value_type&&>(val))
					   : std::dynamic_pointer_cast<stored_element>(val);
		if (!ptr)
			return false;

		out.push_back({std::move(ptr), stored_service_cast_tag{}});

		return steal;
	});

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
	auto& loader = services_loader::get( );
	if (this->root_class( ))
		reload_one_instance(loader);
	else
		loader.erase(this->type( ));
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
	return loader->find(info);
}
