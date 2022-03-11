module;

#include <nstd/ranges.h>
#include <nstd/runtime_assert.h>

#include <functional>
#include <future>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

module cheat.hooks:loader;
import cheat.console.object_message;
import nstd.one_instance;

using namespace cheat;
using console::object_message;
using namespace nstd;

class storage_t :object_message<storage_t>
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
std::string_view object_message<storage_t>::_Name( )const
{
	return "hooks::storage";
}
static one_instance_obj<storage_t> storage;

static auto debug_thread_id( )
{
	std::ostringstream s;
	s << "thread 0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << std::this_thread::get_id( );
	return std::move(s).str( );
}

template<class T>
static void swap_instant(T& from)
{
	T to;
	using std::swap;
	swap(from, to);
}

class simple_thread_pool :object_message<simple_thread_pool>
{
	void _Already_started_assert( )const
	{
		runtime_assert(pos_ == 0, "Already started");
		runtime_assert(threads_.empty( ), "Already started");
	}

	size_t wait_for_threads( )
	{
		size_t joinable = 0;
		for (auto& thr : threads_)
		{
			if (thr.joinable( ))
			{
				++joinable;
				thr.join( );
			}
		}
		return joinable;
	}

	void worker( )
	{
		//const auto id = debug_thread_id( );
		//this->message("{} started", id);
		for (;;)
		{
			auto current_pos = pos_++;
			if (current_pos >= storage_.size( ))
				break;
			std::invoke(storage_[current_pos]);
		}
		//this->message("{} finished", id);
	}

public:
	using thread_type = std::thread;

	simple_thread_pool( ) = default;

	~simple_thread_pool( )
	{
		finish( );
	}

	template<typename T>
	void add(T&& fn)
	{
		_Already_started_assert( );
		storage_.emplace_back(std::forward<T>(fn));
	}

	void start( )
	{
		_Already_started_assert( );
		const auto threads_count = std::min(storage_.size( ), thread_type::hardware_concurrency( ));
		runtime_assert(threads_count > 0, "Incorrect threads count");
		this->message("started");
		threads_.reserve(threads_count);
		while (threads_.size( ) != threads_count)
			threads_.emplace_back(&simple_thread_pool::worker, this);
	}

	void join( )
	{
		if (wait_for_threads( ) > 0)
			this->message("stopped");
	}

	void finish( )
	{
		pos_ = storage_.size( );
		wait_for_threads( );
	}

	void destroy( )
	{
		swap_instant(threads_);
		swap_instant(storage_);
	}

private:
	std::vector<std::function<void( )>> storage_;
	std::atomic<size_t> pos_ = 0;
	std::vector<thread_type> threads_;
};

std::string_view object_message<simple_thread_pool>::_Name( )const
{
	return "hooks::multithread_loader";
}

static one_instance_obj<simple_thread_pool> async_loader;

static std::atomic<bool> load_error = false;

void register_hook(hook_creator&& creator)
{
	async_loader->add([creator1 = std::move(creator), holder = storage->write( )]( )
	{
		if (load_error == false)
		{
			auto hook = std::invoke(creator1);
			if (hook->hook( ) && hook->enable( ))
				*holder = std::move(hook);
			else
				load_error = true;
		}
	});
}

std::future<bool> hooks::start( )
{
	//console::log(debug_thread_id());
	constexpr auto load = []
	{
		//console::log(debug_thread_id());
		async_loader->start( );
		async_loader->join( );
		async_loader->destroy( );
		return load_error == false;
	};

#if 1
	auto prom = std::make_shared<std::promise<bool>>( );
	std::thread([=]
	{
		prom->set_value(load( ));
	}).detach( );
	return prom->get_future( );
#else
	return std::async(std::launch::async, load);
#endif
}

void hooks::stop(bool force)
{
	if (force)
	{
		async_loader->finish( );
		async_loader->destroy( );
		storage->clear( );
	}
	else
	{
		async_loader->join( );
		for (auto& h : *storage)
			h->request_disable( );
	}
}