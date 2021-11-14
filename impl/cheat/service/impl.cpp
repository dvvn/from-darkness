#include "impl.h"
#include "state.h"
#include "stored.h"

#include <nstd/runtime_assert_fwd.h>

#include <cppcoro/when_all.hpp>
#include <cppcoro/async_mutex.hpp>
#include <cppcoro/static_thread_pool.hpp>
#include <cppcoro/task.hpp>

#include <ranges>

using namespace cheat;

#ifdef _DEBUG
#define CHEAT_SERVICE_LOADING_ASSERT(_STATE_,...) \
	{ __VA_ARGS__;\
	const service_state _state = _STATE_;\
	runtime_assert(_state != service_state::loading && _state != service_state::waiting, "Unable to modify running service!");}
#else
CHEAT_SERVICE_LOADING_ASSERT(...) (void)0
#endif

struct basic_service::impl
{
	cppcoro::async_mutex lock;
	std::vector<stored_service<basic_service>> deps;
	service_state state = service_state::unset;
};

basic_service::basic_service( )
{
	impl_ = std::make_unique<impl>( );
}

basic_service::~basic_service( )
{
	CHEAT_SERVICE_LOADING_ASSERT(impl_->state, if(!impl_) return);
}

basic_service::basic_service(basic_service&& other) noexcept
{
	*this = std::move(other);
}

basic_service& basic_service::operator=(basic_service&& other) noexcept
{
	CHEAT_SERVICE_LOADING_ASSERT(impl_->state);
	CHEAT_SERVICE_LOADING_ASSERT(other.impl_->state);

	std::swap(impl_, other.impl_);

	return *this;
}

auto basic_service::find(const std::type_info& info) const -> const value_type*
{
	return std::_Const_cast(this)->find(info);
}

auto basic_service::find(const std::type_info& info) -> value_type*
{
	for (auto& d: impl_->deps)
	{
		auto& info1 = d->type( );
		if (info == info1)
			return std::addressof(d);
	}
	return nullptr;
}

template <class A, class B, typename ...Args>
static constexpr decltype(auto) _Select_one(A&& a, B&& b, Args&& ...args)
{
	if constexpr (std::invocable<A, Args...>)
		return std::invoke(std::forward<A>(a), std::forward<Args>(args)...);
	else if constexpr (std::invocable<B, Args...>)
		return std::invoke(std::forward<B>(b), std::forward<Args>(args)...);
}

template <bool Once, typename T, typename Pred, typename Proj = std::identity>
static auto _Erase_impl(T& from, Pred&& pred, Proj proj = {})
{
	namespace rng = std::ranges;
	if constexpr (Once)
	{
		auto itr = _Select_one(rng::find_if, rng::find, from, pred, proj);
		if (itr != from.end( ))
			return from.erase(itr);
	}
	else
	{
		auto to_remove = _Select_one(rng::remove_if, rng::remove, from, pred, proj);
		if (!to_remove.empty( ))
			return from.erase(to_remove.begin( ), to_remove.end( ));
	}

	return from.end( );
}

// ReSharper disable once CppMemberFunctionMayBeConst
void basic_service::erase(const std::type_info& info)
{
	_Erase_impl<true>(impl_->deps, info, &basic_service::type);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void basic_service::erase(const erase_pred& fn)
{
	_Erase_impl<true>(impl_->deps, std::_Pass_fn(fn));
}

// ReSharper disable once CppMemberFunctionMayBeConst
void basic_service::erase_all(const erase_pred& fn)
{
	_Erase_impl<false>(impl_->deps, std::_Pass_fn(fn));
}

// ReSharper disable once CppMemberFunctionMayBeConst
void basic_service::add_dependency(value_type&& srv)
{
	runtime_assert(!find(srv->type()), "Service already stored!");
	impl_->deps.push_back(std::move(srv));
}

service_state basic_service::state( ) const
{
	return impl_->state;
}

auto basic_service::load(executor& ex) noexcept -> load_result
{
	switch (impl_->state)
	{
		case service_state::unset:
		{
			const auto mtx = co_await impl_->lock.scoped_lock_async( );
			//check is other thread do all the work
			switch (impl_->state)
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
				if (impl_->deps.empty( ))
					co_return true;

				using std::views::transform;
				using std::ranges::all_of;

				impl_->state = service_state::waiting;
				co_await ex.schedule( );
				const auto unwrapped = impl_->deps | transform([](const auto& srv)-> basic_service& { return *srv; });
				auto tasks_view      = unwrapped | transform([&](basic_service& srv)-> load_result { return srv.load(ex); });
				auto results         = co_await when_all(std::vector(tasks_view.begin( ), tasks_view.end( )));
				co_return all_of(results, [](bool val) { return val == true; });
			};

			if (!co_await load_deps( ))
			{
				runtime_assert("Unable to load other services!");
				impl_->state = service_state::error;
				co_return false;
			}

			//-----------

			const auto load_this = [&]( )-> load_result
			{
				impl_->state = service_state::loading;
				co_await ex.schedule( );
				co_return co_await this->load_impl( );
			};

			//-----------

			if (!co_await load_this( ))
			{
				runtime_assert("Unable to load service!");
				impl_->state = service_state::error;
				co_return false;
			}

			//-----------

			impl_->state = service_state::loaded;
			co_return true;
		}
		case service_state::waiting:
		case service_state::loading:
		{
			//some other thread loads this service,
			const auto mtx = co_await impl_->lock.scoped_lock_async( );
			switch (impl_->state)
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
