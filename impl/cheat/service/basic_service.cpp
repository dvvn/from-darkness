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

auto basic_service::load_deps(const executor ex, const callback_type callback) noexcept-> task_type
{
	if (load_before.empty( ))
		co_return true;

	const auto error = std::make_shared<std::atomic_bool>(false);
	const callback_type callback_wrapped = [=](basic_service* caller, state_type result)
	{
		callback(caller, result);

		if (result != state_type::loaded_error)
			return;
		*error = true;
	};
	const auto start_wrapped = [=](deps_storage::value_type& srv)->task_type
	{
		if (*error)
		{
			static const auto false_task = std::invoke([]( )->task_type { co_return false; });
			return false_task;
		}

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
	callback(this, state);
	co_return loaded;
}

auto basic_service::start_impl(const executor ex, const callback_type callback) noexcept -> task_type
{
	construct( );
	co_return co_await load_deps(ex, callback) && co_await load_this(ex, callback);
}

void basic_service::start(const executor ex, const callback_type callback) noexcept
{
	if (state != state_type::idle)
		return;

	state = state_type::started;
	start_result = start_impl(ex, callback);
}

void basic_service::start(const executor ex) noexcept
{
	static const callback_type default_callback
#ifdef _DEBUG
		= [](basic_service* caller, basic_service::state_type result)
	{
		[[maybe_unused]]
		uint8_t debugger_gap = 0;
	}
#endif
	;

	this->start(ex, default_callback);
}

void basic_service::start( ) noexcept
{
	this->start(executor( ), log_service_start);
}
