module;

#include <nstd/ranges.h>
#include <nstd/runtime_assert.h>

#include <thread_pool.hpp>

#include <functional>
#include <future>
#include <vector>
#include <list>
#include <string_view>

module cheat.hooks.loader;
import cheat.console.lifetime_notification;
import nstd.one_instance;
import nstd.lazy_invoke;

using namespace cheat;
using console::lifetime_notification;
using namespace nstd;

class storage_t :lifetime_notification<storage_t>
{
	std::list<stored_hook> storage_;
public:
	storage_t( ) = default;

	auto write( )
	{
		return std::addressof(storage_.emplace_back( ));
	}

	void clear( )
	{
		storage_.clear( );
	}

	auto begin( )
	{
		return storage_.begin( );
	}

	auto end( )
	{
		return storage_.end( );
	}
};
std::string_view lifetime_notification<storage_t>::_Name( )const
{
	return "hooks::storage";
}
static one_instance_obj<storage_t> storage;

class async_loader_t : thread_pool, lifetime_notification<async_loader_t>
{
	std::unique_ptr<thread_pool> pool;

public:
	async_loader_t( )
	{
		pool = std::make_unique<thread_pool>( );
		pool->sleep_duration = 0;
		pool->paused = true;

	}

	template<typename Fn>
	void push_task(const Fn& fn)
	{
		pool->push_task(fn);
	}

	void run( )
	{
		pool->paused = false;
		this->message("started");
	}

	void pause( )
	{
		pool->paused = true;
		pool->wait_for_tasks( );
	}

	void finish( )
	{
		if (!pool)
			return;
		pool.reset( );
		this->message("finished");
	}
};

std::string_view lifetime_notification<async_loader_t>::_Name( )const
{
	return "hooks::multithread_loader";
}

static one_instance_obj<async_loader_t> async_loader;

static std::atomic<bool> load_error = false;

void register_hook(hook_creator&& creator)
{
	async_loader->push_task([creator1 = std::move(creator), holder = storage->write( )]( )
	{
		if (load_error.load(std::memory_order_relaxed) == false)
		{
			auto hook = std::invoke(creator1);
			if (hook->hook( ) && hook->enable( ))
				*holder = std::move(hook);
			else
				load_error.store(true, std::memory_order_relaxed);
		}
	});
}

std::future<bool> hooks::start( )
{
	async_loader->run( );
	return std::async(std::launch::async, []
	{
		async_loader->finish( );
		return load_error.load(std::memory_order_relaxed) == false;
	});
}

void hooks::stop(bool force)
{
	if (force)
	{
		async_loader->finish( );
		storage->clear( );
	}
	else
	{
		async_loader->pause( );
		for (auto& h : *storage)
			h->request_disable( );
	}
}