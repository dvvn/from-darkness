module;

#include <nstd/ranges.h>
#include <nstd/runtime_assert.h>

#include <thread_pool.hpp>

#include <functional>
#include <future>
#include <vector>
#include <string_view>

module cheat.hooks.loader;
import cheat.console.lifetime_notification;
import nstd.one_instance;
import nstd.lazy_invoke;

using namespace cheat::hooks;
using cheat::console::lifetime_notification;
using namespace nstd;

struct pending_storage_t : std::vector<hook_creator>, lifetime_notification<pending_storage_t>
{
	pending_storage_t( ) = default;

	auto unpack( )
	{
		return *this | std::views::transform([](hook_creator& creator)
		{
#ifdef _DEBUG
			const nstd::lazy_invoke lazy = [&creator]
			{
				using std::swap;
				hook_creator dummy;
				swap(creator, dummy);
			};
#endif
			return std::invoke(creator);
		});
	}
};

std::string_view lifetime_notification<pending_storage_t>::_Name( )const
{
	return "hooks::pending_storage";
}

#if 0
void one_instance_obj<pending_storage_t>::_Recreate( )const
{
	auto& vec = **this;
	vec.clear( );
	vec.shrink_to_fit( );
}
#endif
static one_instance_obj<pending_storage_t> pending_storage;

struct storage_t :std::vector<stored_hook>, lifetime_notification<storage_t>
{
	storage_t( ) = default;
};
std::string_view lifetime_notification<storage_t>::_Name( )const
{
	return "hooks::storage";
}
static one_instance_obj<storage_t> storage;

void register_hook(hook_creator&& creator)
{
	runtime_assert(storage->empty( ));
	pending_storage->push_back(std::move(creator));
}

bool loader::start( )
{
	if (storage->empty( ))
	{
		auto filler = pending_storage->unpack( );
		storage->assign(filler.begin( ), filler.end( ));
	}

	for (auto& hook : *storage)
	{
		if (!hook->hook( ) || !hook->enable( ))
			return false;
	}

	return true;
}

struct async_loader_t :thread_pool, lifetime_notification<async_loader_t>
{
	async_loader_t( ) = default;
};

std::string_view lifetime_notification<async_loader_t>::_Name( )const
{
	return "hooks_multithread_loader";
}

static one_instance_obj<async_loader_t> async_loader;

std::future<bool> loader::start_async( )
{
	const auto size = pending_storage->size( );
	if (storage->empty( ))
		storage->resize(size);

	auto error = std::make_shared<std::atomic_bool>(false);
	for (size_t i = 0; i < size; ++i)
	{
		async_loader->push_task([=]
		{
			if (error->load(std::memory_order_relaxed) == true)
				return;

			auto& from = pending_storage->data( )[i];
			auto& to = storage->data( )[i];

			auto hook = std::invoke(from);
			if (!hook->hook( ) || !hook->enable( ))
				error->store(true, std::memory_order_relaxed);
			else
				to = std::move(hook);
		});
	}

	auto prom = std::make_shared<std::promise<bool>>( );
	async_loader->push_task([=/*,call_later=nstd::lazy_invoke(pending_storage::get( ))*/]
	{
		prom->set_value(error->load(std::memory_order_relaxed) == false);
		std::async(std::launch::async, [=]
		{
			pending_storage._Recreate( );
			auto& pool = static_cast<thread_pool&>(*async_loader);
			pool.~thread_pool( );
		});
	});
	return prom->get_future( );
}

void loader::stop(bool force)
{
	if (force)
	{
		pending_storage->clear( );
		storage->clear( );
	}
	else
	{
		for (auto& h : *storage)
			h->request_disable( );
	}
}