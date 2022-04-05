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
using hooks::hook_data;

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

class hooks_loader;
std::string_view object_message_impl<hooks_loader>::get_name( ) const
{
	return "hooks::loader";
}

class hooks_loader :object_message_auto<hooks_loader>
{
	void _Already_started_assert( )const
	{
		runtime_assert(pos_ == 0, "Already started");
		runtime_assert(threads_.empty( ), "Already started");
	}

	using error_t = std::shared_ptr<std::atomic<bool>>;

	void worker(const error_t error)
	{
		//const auto id = debug_thread_id( );
		//this->message("{} started", id);
		for (;;)
		{
			if (*error)
				break;
			const auto current_pos = pos_++;
			if (current_pos >= storage_.size( ))
				break;
			if (!storage_[current_pos].start( ))
			{
				*error = true;
				break;
			}
		}
		//this->message("{} finished", id);
	}

public:
	using thread_type = std::thread;

	hooks_loader( ) = default;

	~hooks_loader( )
	{
		finish( );
	}

	void add(hook_data data)
	{
		_Already_started_assert( );
		storage_.push_back(data);
	}

	error_t start( )
	{
		_Already_started_assert( );
		const auto threads_count = std::min(storage_.size( ), thread_type::hardware_concurrency( ));
		runtime_assert(threads_count > 0, "Incorrect threads count");
		this->message("started");
		threads_.reserve(threads_count);
		auto error = std::make_shared<error_t::element_type>( );
		while (threads_.size( ) != threads_count)
			threads_.emplace_back(&hooks_loader::worker, this, error);
		return error;
	}

	bool join( )
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
		if (joinable == 0)
			return false;

		this->message("stopped");
		return true;
	}

	void finish( )
	{
		const auto last_pos = std::min<size_t>(pos_, storage_.size( ));
		pos_ = storage_.size( );
		this->join( );
		for (size_t i = 0; i < last_pos; ++i)
			storage_[i].stop( );
	}

private:
	std::vector<hook_data> storage_;
	std::atomic<size_t> pos_ = 0;
	std::vector<thread_type> threads_;
};

static one_instance_obj<hooks_loader> loader;

void hooks::add(hook_data data)
{
	loader->add(data);
}

std::future<bool> hooks::start( )
{
	//console::log(debug_thread_id());
	constexpr auto load = []
	{
		//console::log(debug_thread_id());
		const auto error = loader->start( );
		loader->join( );
		return *error == false;
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

void hooks::stop( )
{
	loader->finish( );
	/*for (auto& h : *storage)
		h->request_disable( );*/
}