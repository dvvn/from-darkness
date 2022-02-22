module;

#include "basic_includes.h"

#include <cppcoro/sync_wait.hpp>

module cheat.service:basic;
import :tools;
import nstd.container.wrapper;

using namespace cheat;

basic_service::basic_service( ) = default;
basic_service::~basic_service( ) = default;

basic_service::basic_service(basic_service && other) noexcept
{
	*this = std::move(other);
}

//template<typename T>
//static void atomic_swap(std::atomic<T>&l, std::atomic<T>&r)
//{
//	T tmp = l;
//	l = static_cast<T>(r);
//	r = tmp;
//}

namespace std
{
	template<typename T>
	void swap(std::atomic<T>& l, std::atomic<T>& r)noexcept
	{
		T tmp = l;
		l = static_cast<T>(r);
		r = tmp;
	}
}

basic_service& basic_service::operator=(basic_service && other) noexcept
{
	using std::swap;
	using cppcoro::swap;
	swap(state, other.state);
	swap(start_result, other.start_result);
	swap(load_before, other.load_before);
	return *this;
}

size_t basic_service::deps_storage::add(value_type && val)
{
	runtime_assert(!this->contains(val->type( )), "Service already stored!");
	this->push_back(std::move(val));
	return this->size( ) - 1;
}

size_t basic_service::deps_storage::add(const value_type & val)
{
	runtime_assert(!this->contains(val->type( )), "Service already stored!");
	this->push_back(val);
	return this->size( ) - 1;
}

auto basic_service::deps_storage::find(const std::type_info & type) -> iterator
{
	const auto end = this->end( );
	for (auto itr = this->begin( ); itr != end; ++itr)
	{
		auto& smart_ptr = *itr;
		auto& val = *smart_ptr;
		if (val.type( ) == type)
			return itr;
	}
	return end;
}

auto basic_service::deps_storage::find(const std::type_info & type) const ->const_iterator
{
	return const_cast<deps_storage*>(this)->find(type);
}

bool  basic_service::deps_storage::contains(const std::type_info & type) const
{
	return this->find(type) != this->end( );
}

struct wrapped_callback : basic_service::callback_type
{
	/*basic_service::callback_type prev_callback;
	std::atomic<bool> error = false;

	wrapped_callback(const basic_service::callback_type& cb) :prev_callback(cb)
	{
	}

	void operator()(basic_service* caller, basic_service::state_type result) noexcept
	{
		std::invoke(*prev_callback, caller, result);

		if (result != state_type::loaded_error)
			return;
		error = true;
	}*/
};

static void _Invoke_callback(const basic_service::callback_type & callback, basic_service * caller, basic_service::state_type result)
{
#ifndef _DEBUG
	if (!callback)
		return;
#endif
	std::invoke(*callback, caller, result);
}

auto basic_service::load_deps(const executor ex, const callback_type callback) noexcept-> task_type
{
	if (load_before.empty( ))
		co_return true;

	static auto false_task = std::invoke([]( )->task_type { co_return false; });

	const auto error = std::make_shared<std::atomic_bool>(false);
	const auto callback_wrapped = std::make_shared<callback_type::element_type>([=](basic_service* caller, state_type result)
	{
		_Invoke_callback(callback, caller, result);

		if (result != state_type::loaded_error)
			return;
		*error = true;
	});
	const auto start_wrapped = [=](value_type& srv)->task_type
	{
		if (*error)
			return false_task;

		srv->start(ex, callback_wrapped);
		return srv->start_result;
	};

	co_await ex->schedule( );

	using tasks_rng_t = std::vector<task_type>;
	const auto results = co_await cppcoro::when_all(nstd::container::append<tasks_rng_t>(load_before | std::views::transform(start_wrapped)));
	if (*error)
		co_return false;

	co_return std::ranges::all_of(results, std::bind_front(std::equal_to<bool>( ), true));
}

auto basic_service::load_this(const executor ex, const callback_type callback) noexcept-> task_type
{
	co_await ex->schedule( );
	const auto loaded = this->load( );
	state = loaded ? state_type::loaded : state_type::loaded_error;
	_Invoke_callback(callback, this, state);
	co_return loaded;
}

auto basic_service::start_impl(const executor ex, const callback_type callback) noexcept -> task_type
{
	this->construct( );
	co_return co_await load_deps(ex, callback) && co_await load_this(ex, callback);
}

void basic_service::start(const executor ex, const callback_type callback) noexcept
{
	if (state == state_type::idle)
	{
		state = state_type::started;
		start_result = start_impl(ex, callback);
	}
}

void basic_service::start(const executor ex) noexcept
{
	callback_type default_callback;
#ifdef _DEBUG
	default_callback = std::make_shared<callback_type::element_type>([](basic_service* caller, state_type result)
	{
		uint8_t debugger_gap = 0;
	});
#endif
	start(ex, std::move(default_callback));
}

void basic_service::start( ) noexcept
{
	start(std::make_shared<executor::element_type>( ), std::make_shared<callback_type::element_type>(log_service_start));
}
