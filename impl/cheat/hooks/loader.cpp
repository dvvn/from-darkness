module;

#include <nstd/ranges.h>
#include <thread_pool.hpp>

#include <functional>
#include <future>
#include <vector>
#include <string_view>

export module cheat.hooks.loader;
import cheat.console.lifetime_notification;
//import nstd.one_instance;
import nstd.lazy_invoke;

using namespace cheat::hooks;
using cheat::console::lifetime_notification;
using nstd::one_instance;

struct pending_storage : std::vector<hook_creator>, lifetime_notification<pending_storage>, one_instance<pending_storage>
{
};
struct storage :std::vector<stored_hook>, lifetime_notification<storage>, one_instance<storage>
{
};

std::string_view lifetime_notification<pending_storage>::_Name( )
{
	return "pending_hooks_storage";
}

std::string_view lifetime_notification<storage>::_Name( )
{
	return "hooks_storage";
}

//static pending_storage_t pending_storage;
//static hooks_storage_t hooks_storage;

void register_hook(hook_creator&& creator)
{
	runtime_assert(storage::get( ).empty( ));
	pending_storage::get( ).push_back(std::move(creator));
}

static auto _Pending_filler( )
{
	return pending_storage::get( ) | std::views::transform([](hook_creator& creator)
	{
#ifdef _DEBUG
		using std::swap;
		hook_creator dummy;
		const nstd::lazy_invoke lazy = [&]
		{
			swap(creator, dummy);
		};
#endif
		return std::invoke(creator);
	});
}

bool loader::start( )
{
	if (storage::get( ).empty( ))
	{
		auto filler = _Pending_filler( );
		storage::get( ).assign(filler.begin( ), filler.end( ));
		pending_storage::_Reload( );
	}

	for (auto& hook : storage::get( ))
	{
		if (!hook->hook( ) || !hook->enable( ))
			return false;
	}

	return true;
}

struct async_loader :thread_pool, lifetime_notification<async_loader>, one_instance<async_loader>
{
};

std::string_view lifetime_notification<async_loader>::_Name( )
{
	return "hooks_multithread_loader";
}

std::future<bool> loader::start_async( )
{
	if (storage::get( ).empty( ))
	{
		const auto size = pending_storage::get( ).size( );
		storage::get( ).resize(size);

		auto error = std::make_shared<std::atomic_bool>(false);
		for (size_t i = 0; i < size; ++i)
		{
			async_loader::get( ).push_task([=]
			{
				if (error->load(std::memory_order_relaxed))
					return;

				auto& from = pending_storage::get( )[i];
				auto& to = storage::get( )[i];

				auto hook = std::invoke(from);
				if (!hook->hook( ) || !hook->enable( ))
					error->store(true, std::memory_order_relaxed);
				else
					to = std::move(hook);
			})
		}

		auto prom = std::make_shared<std::promise<bool>>( );
		async_loader::get( ).push_task([=]
		{
			prom.set_value(error->load(std::memory_order_relaxed) == false);
		});
		return prom->get_future( );
	}
	else
	{
		//wip
	}
}

void loader::stop(bool force)
{
	if (force)
	{
		for (auto& h : hooks_storage)
			h->disable( );
	}
	else
	{
		for (auto& h : hooks_storage)
			h->request_disable( );
	}
}