module;

#include <nstd/ranges.h>
#include <nstd/runtime_assert.h>

#include <functional>
#include <future>
#include <vector>
#include <list>
#include <string>
#include <sstream>
#include <iomanip>

module cheat.hooks:loader;
import cheat.console.object_message;
import nstd.one_instance;

using namespace cheat;
using namespace console;
using namespace nstd;

static auto debug_thread_id( ) noexcept
{
	std::ostringstream s;
	s << "thread 0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << std::this_thread::get_id( );
	return std::move(s).str( );
}

class debug_thread_id_lazy
{
	std::string id_;

public:
	std::string_view operator()( ) noexcept
	{
		if(id_.empty( ))
			id_ = debug_thread_id( );
		return id_;
	}
};

template<class T>
static void swap_instant(T& from)
{
	T to;
	using std::swap;
	swap(from, to);
}

class hooks_loader;
std::string_view object_message_impl<hooks_loader>::get_name( ) const noexcept
{
	return "hooks::loader";
}

//#define WORKER_DATA_COUNT_THREADS
//#define WORKER_LOG_THREAD_ID

struct hooks_worker_data
{
	std::atomic<bool> error = false;
	std::atomic<size_t> pos = 0;
#ifdef WORKER_DATA_COUNT_THREADS
	std::atomic<size_t> threads;

	hooks_worker_data(size_t threads_count)
		:threads(threads_count)
	{
	}
#endif
};
using hooks::hook_data;

class hooks_loader
{
	[[no_unique_address]]
	object_message_auto<hooks_loader> msg_;

	using worker_data = std::shared_ptr<hooks_worker_data>;

	void worker(const worker_data data) noexcept
	{
#ifdef WORKER_LOG_THREAD_ID
		debug_thread_id_lazy id;
		msg_("{} started", id);
#endif
		auto& [error
			, pos
#ifdef WORKER_DATA_COUNT_THREADS
			, threads
#endif
		] = *data;

		for(;;)
		{
			if(error)
				break;
			const auto current_pos = pos++;
			if(current_pos >= storage_.size( ))
				break;
			if(!storage_[current_pos].start( ))
			{
				error = true;
				break;
			}
		}

#ifdef WORKER_DATA_COUNT_THREADS
		--threads;
#endif

#ifdef WORKER_LOG_THREAD_ID
		msg_("{} finished", id);
#endif
	}

public:
	using thread_type = std::thread;

	hooks_loader( ) = default;

	~hooks_loader( )
	{
		stop( );
	}

	void add(hook_data data) noexcept
	{
		runtime_assert(threads_.empty( ), "Already started");
		storage_.push_back(data);
	}

	worker_data start( ) noexcept
	{
		this->join( );

		const auto threads_count = std::min(storage_.size( ), thread_type::hardware_concurrency( ));
		runtime_assert(threads_count > 0, "Incorrect threads count");

		msg_("started");

		threads_.reserve(threads_count);
		auto wdata = std::make_shared<hooks_worker_data>(
#ifdef WORKER_DATA_COUNT_THREADS
			threads_count
#endif
			);
		while(threads_.size( ) != threads_count)
			threads_.emplace_back(&hooks_loader::worker, this, wdata);
		return wdata;
	}

	bool join( ) noexcept
	{
		if(threads_.empty( ))
			return false;

		size_t joinable = 0;
		for(auto& thr : threads_)
		{
			if(thr.joinable( ))
			{
				++joinable;
				thr.join( );
			}
		}

		swap_instant(threads_);

		if(joinable == 0)
			return false;

		msg_("stopped");
		return true;
	}

	void stop( ) noexcept
	{
		this->join( );
		for(auto& entry : storage_)
			entry.stop( );
	}

private:
	std::vector<hook_data> storage_;
	std::vector<thread_type> threads_;
};

static instance_of_t<hooks_loader> loader;

void hooks::add(hook_data data) noexcept
{
	loader->add(data);
}

static bool start_impl( ) noexcept
{
	//console::log(debug_thread_id());
	const auto data = loader->start( );
	loader->join( );
	return data->error == false;
}

std::future<bool> hooks::start( ) noexcept
{
	//console::log(debug_thread_id());

#if 1
	auto prom = std::make_shared<std::promise<bool>>( );
	std::thread([=]
	{
		prom->set_value(start_impl( ));
	}).detach( );
	return prom->get_future( );
#else
	return std::async(std::launch::async, start_impl);
#endif
}

void hooks::stop( ) noexcept
{
	loader->stop( );
}

//bool hooks::active( ) noexcept
//{
//	return loader->active( );
//}