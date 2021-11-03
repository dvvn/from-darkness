#include "impl.h"
#include "data.h"

#include <nstd/runtime_assert_fwd.h>

#include <cppcoro/when_all.hpp>
#include <cppcoro/async_mutex.hpp>
#include <cppcoro/static_thread_pool.hpp>
#include <cppcoro/task.hpp>

#include <ranges>

using namespace cheat;

template <typename T>
static CPPCORO_FORCE_INLINE void _Loading_access_assert([[maybe_unused]] T&& state)
{
	runtime_assert(state != service_state::loading && state != service_state::waiting, "Unable to modify running service!");
}

struct basic_service::lock_impl : cppcoro::async_mutex
{
};

basic_service::basic_service( )
{
	lock_  = std::make_unique<lock_impl>( );
	deps_  = std::make_unique<basic_service_data>( );
	state_ = service_state::unset;
}

basic_service::~basic_service( )
{
	_Loading_access_assert(state_);
}

basic_service::basic_service(basic_service&& other) noexcept
{
	*this = std::move(other);
}

basic_service& basic_service::operator=(basic_service&& other) noexcept
{
	_Loading_access_assert(state_);
	_Loading_access_assert(other.state_);

	std::swap(state_, other.state_);
	std::swap(lock_, other.lock_);
	std::swap(deps_, other.deps_);

	return *this;
}

auto basic_service::find_service(const std::type_info& info) const -> const value_type*
{
	return std::_Const_cast(this)->find_service(info);
}

auto basic_service::find_service(const std::type_info& info) -> value_type*
{
	for (auto& d: *deps_)
	{
		auto& info1 = d->type( );
		if (info == info1)
			return std::addressof(d);
	}
	return nullptr;
}

// ReSharper disable once CppMemberFunctionMayBeConst
void basic_service::add_dependency(value_type&& srv)
{
	runtime_assert(!find_service(srv->type()), "Service already stored!");
	deps_->push_back(std::move(srv));
}

service_state basic_service::state( ) const
{
	return state_;
}

auto basic_service::load(executor& ex) noexcept -> load_result
{
	switch (state_)
	{
		case service_state::unset:
		{
			const auto mtx = co_await lock_->scoped_lock_async( );
			//check is other thread do all the work
			switch (state_)
			{
				case service_state::loaded:
					co_return true;
				case service_state::error:
					co_return false;
				case service_state::unset:
					break;
				default:
					runtime_assert("Preload: mutex released, but state isnt't sets correctly!");
					std::terminate( );
			}

			//-----------

			const auto load_deps = [&]( )-> load_result
			{
				if (deps_->empty( ))
					co_return true;

				using std::views::transform;
				using std::ranges::all_of;

				state_ = service_state::waiting;
				co_await ex.schedule( );
				const auto unwrapped = *deps_ | transform([](const auto& srv)-> basic_service& { return *srv; });
				auto tasks_view      = unwrapped | transform([&](basic_service& srv)-> load_result { return srv.load(ex); });
				auto results         = co_await when_all(std::vector(tasks_view.begin( ), tasks_view.end( )));
				co_return all_of(results, [](bool val) { return val == true; });
			};

			if (!co_await load_deps( ))
			{
				runtime_assert("Unable to load other services!");
				state_ = service_state::error;
				co_return false;
			}

			//-----------

			const auto load_this = [&]( )-> load_result
			{
				state_ = service_state::loading;
				co_await ex.schedule( );
				co_return co_await this->load_impl( );
			};

			//-----------

			if (!co_await load_this( ))
			{
				runtime_assert("Unable to load service!");
				state_ = service_state::error;
				co_return false;
			}

			//-----------

			state_ = service_state::loaded;
			co_return true;
		}
		case service_state::waiting:
		case service_state::loading:
		{
			//some other thread loads this service,
			const auto mtx = co_await lock_->scoped_lock_async( );
			switch (state_)
			{
				case service_state::loaded:
					co_return true;
				case service_state::error:
					co_return false;
				default:
					runtime_assert("Postload: mutex released, but state isnt't sets correctly!");
					std::terminate( );
			}
		}
		case service_state::loaded:
		{
			co_return true;
		}
		case service_state::error:
		{
			runtime_assert("Service loaded with errors!");
			co_return false;
		}
		default:
		{
			runtime_assert("Unknown state");
			co_return false;
		}
	}
}
