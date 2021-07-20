#include "service.h"

#include "console.h"

using namespace cheat;
using namespace detail;
using namespace utl;
using namespace future_state;

service_base::~service_base( )
{
	Waiting_task_assert_( );
}

service_base::service_base( )
{
	creator__ = this_thread::get_id( );
}

future<void> _Get_ready_task( )
{
	promise<void> pr;
	pr.set_value( );
	return pr.get_future( );
}

template <std::derived_from<std::exception> Ex>
future<void> _Get_error_task(Ex&& ex)
{
	promise<void> pr;
	pr.set_exception(move(ex));
	return pr.get_future( );
}

void service_base::reset( )
{
	Waiting_task_assert_( );
	if(!load_task__.is_ready( ))
		load_task__ = _Get_ready_task( );
	wait_for__.clear( );
}

bool service_base::initialized( ) const
{
	return load_task__.is_ready( );
}

service_base::load_task_type service_base::init(loader_type& loader)
{
	switch(load_task__.get_state( ))
	{
	case uninitialized:
		if(loader.closed( ))
			load_task__ = _Get_error_task(std::runtime_error("Loader is closed!"));
		else
			load_task__ = this->Initialize(loader);
#if 0
		if(!load_synchronously_)
			load_task__ = async(loader, Init_internal_(loader));
		else
		{
			init_this( );
			load_task__ = _Get_ready_task( );
		}
#endif
	case waiting:
	case ready:
		return load_task__;
	case moved:
		BOOST_ASSERT("Task is moved");
		return _Get_ready_task( );
	case deferred:
		BOOST_ASSERT("Task is deferred");
		return _Get_error_task(std::runtime_error("Task is deferred"));
	default:
		BOOST_ASSERT("Task state unknown");
		std::terminate( );
	}
}

template <typename ...T>
static string _Concat_str(T&& ...str)
{
	std::ostringstream stream;
	((stream << forward<T>(str)), ...);
	return stream.str( );
}

static constexpr string_view loaded_msg = "Service loaded";
static constexpr string_view skipped_msg = "Service skipped";

template <typename T, typename ...Ts>
static string _Get_loaded_message(const service_base* owner, T&& msg, Ts&& ...msg_other)
{
	auto msg_str = string(forward<T>(msg));
	auto msg_size = msg_str.size( );

	//msg_str += ' ';

	const auto arr = array{msg_size, string_view(msg_other).size( )...};
	auto max = ranges::max(arr);
	if(msg_size != max)
	{
		auto min = ranges::min(arr);
		auto diff = max - min;
		msg_str.append(diff, ' ');
	}

	msg_str += ": ";
	msg_str += owner->debug_name( );

	return move(msg_str);
}

string service_base::Get_loaded_message( ) const
{
	return _Get_loaded_message(this, loaded_msg, skipped_msg);
}

string service_base::Get_loaded_message_disabled( ) const
{
	return _Get_loaded_message(this, skipped_msg, loaded_msg);
}

void service_base::Post_load( )
{
	(void)this;
}

//service_base::wait_for_storage_type& service_base::Storage( )
//{
//	return wait_for__;
//}

void service_base::Print_loaded_message_( ) const
{
#ifdef CHEAT_DEBUG_MODE
	if(const auto msg = Get_loaded_message( ); !msg.empty( ))
		console::get( ).write_line(msg);
#else
	(void)this;
#endif
}

void service_base::Wait_for_add_impl_(service_shared&& service)
{
	BOOST_ASSERT_MSG(creator__ == this_thread::get_id( ), "Unable to modify service from other thread!");
	BOOST_ASSERT_MSG(service->load_task__.get_state( ) == uninitialized, "Unable to add service while load task is set!");
#if 0
	BOOST_ASSERT_MSG(!Find_recursuve_(service), "Some internal service already wait for addable service");
#endif
	BOOST_ASSERT_MSG(!wait_for__.contains(service), "Service already added!");

	wait_for__.emplace(move(service));
}

bool service_base::Find_recursuve_(const service_shared& service) const
{
	for(auto& t : wait_for__)
	{
		if(t->wait_for__.contains(service))
			return true;
		if(t->Find_recursuve_(service))
			return true;
	}

	return false;
}

void service_base::Waiting_task_assert_( ) const
{
	BOOST_ASSERT(load_task__.get_state( ) != waiting);
}

//---------------

service_base::load_task_type async_service::Initialize(loader_type& loader)
{
	vector<load_task_type> tasks_wait_for;
	for(auto& s : wait_for__)
	{
		if(s->load_task__.is_ready( ))
			continue;
		tasks_wait_for.emplace_back(s->init(loader));
	}

	auto load = [this, load_before = move(tasks_wait_for)]( )mutable
	{
		for(auto& t : load_before)
		{
			if(auto ex = t.get_exception_ptr( ))
				rethrow_exception(move(ex));
		}
		this->Load( );
		this->Print_loaded_message_( );
	};

	load_task_type loader_task = async(loader, move(load));

	auto post_load = [this, loader_task]( )mutable
	{
		if(auto ex = loader_task.get_exception_ptr( ))
			rethrow_exception(move(ex));
		this->Post_load( );
	};

	//only loader wait for this task
	loader.submit(move(post_load));

	return loader_task;
}

service_base::load_task_type sync_service::Initialize(loader_type& loader)
{
	try
	{
		for(auto& s : wait_for__)
		{
			switch(auto& task = s->load_task__; task.get_state( ))
			{
			case uninitialized:
				BOOST_ASSERT("Unable to init service before internal services!");
			case ready:
				break;
			default:
				auto ex = task.get_exception_ptr( );
				if(ex)
					rethrow_exception(move(ex));
				break;
			}
			//BOOST_ASSERT_MSG(s->load_task__.get_state() != ready, "Unable to init service before internal services!");
		}

		this->Load( );
		this->Print_loaded_message_( );
		this->Post_load( );
	}
	catch(std::exception&& ex)
	{
		loader.close( );
		return _Get_error_task(move(ex));
	}
	return _Get_ready_task( );
}
