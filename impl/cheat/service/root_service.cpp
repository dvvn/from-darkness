module;
#include "basic_includes.h"

#include <dhooks/includes.h>
#include <windows.h>

module cheat.root_service;
import dhooks;
import nstd.rtlib;

using namespace cheat;
using dhooks::hook_holder_data;

services_loader::services_loader( ) = default;
services_loader::~services_loader( ) = default;

class all_hooks_storage :public services_loader::lazy_reset, basic_service::deps_storage
{
public:
	void try_add(basic_service::value_type& srv, bool steal, bool recursive)
	{
		auto ptr = dynamic_cast<dhooks::hook_disabler_lazy*>(srv.get( ));
		if (!ptr)
			return;
		if (recursive && this->contains(srv->type( )))
			return;
		ptr->request_disable( );
		if (steal)
			this->push_back(std::move(srv));
		else
			this->push_back(srv);
	}
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

static void _Fill_storage(all_hooks_storage * storage, basic_service::deps_storage& deps)
{
	for (auto& d : deps)
	{
		if (!d)
			continue;
		_Fill_storage(storage, d->load_before);
		storage->try_add(d, true, true);
	}
}

template<typename T>
static void _Reset_storage(T & deps)
{
	using val_t = std::ranges::range_value_t<T>;
	static_assert(std::default_initializable<val_t>);

	if constexpr (std::copy_constructible<val_t>)
	{
		val_t val;
		std::ranges::fill(deps, val);
	}
	else
	{
		for (auto& d : deps)
		{
			val_t val;
			using std::swap;
			swap(d, val);
		}
	}
}

auto services_loader::reset(bool deps_only)->reset_object
{
	if (this->state == state_type::idle)
	{
		runtime_assert(std::ranges::count(this->load_before, nullptr) == this->load_before.size( ));
		return nullptr;
	}

	auto hooks = std::make_unique<all_hooks_storage>( );
	_Fill_storage(hooks.get( ), this->load_before);

	if (deps_only)
	{
		this->state = state_type::idle;
		_Reset_storage(this->load_before);
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
