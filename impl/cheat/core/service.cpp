#include "service.h"

#include "console.h"

#include <nstd/runtime_assert_fwd.h>

#include <cppcoro/when_all.hpp>

#include <ranges>

using namespace cheat;

template <typename T>
static void _Loading_access_assert([[maybe_unused]] T&& state)
{
	runtime_assert(state != service_state::loading && state != service_state::waiting, "Unable to modify running service!");
}

struct services_counter
{
	size_t count = 0;
};

size_t service_impl::_Services_count( )
{
	return nstd::one_instance<services_counter>::get_ptr( )->count;
}

service_impl::service_impl( )
{
	++nstd::one_instance<services_counter>::get_ptr( )->count;
}

service_impl::~service_impl( )
{
	_Loading_access_assert(state_);
	--nstd::one_instance<services_counter>::get_ptr( )->count;
}

service_impl::service_impl(service_impl&& other) noexcept
{
	*this = std::move(other);
	++nstd::one_instance<services_counter>::get_ptr( )->count;
}

service_impl& service_impl::operator=(service_impl&& other) noexcept
{
	_Loading_access_assert(this->state_);
	_Loading_access_assert(other.state_);

	std::swap(state_, other.state_);
	std::swap(deps_, other.deps_);

	return *this;
}

auto service_impl::find_service(const std::type_info& info) const -> stored_service
{
	const auto tmp = this->find_service_itr(info);
	return tmp.valid( ) ? *tmp : stored_service( );
}

auto service_impl::find_service_itr(const std::type_info& info) -> iterator_proxy<deps_storage::iterator>
{
	return {std::ranges::find(deps_, info, [&](const stored_service& srv)-> auto& { return srv->type( ); }), std::addressof(deps_)};
}

auto service_impl::find_service_itr(const std::type_info& info) const -> iterator_proxy<deps_storage::const_iterator>
{
	auto tmp = std::_Const_cast(this)->find_service_itr(info);
	return {static_cast<deps_storage::iterator&&>(tmp), std::addressof(deps_)};
}

auto service_impl::add_service_dependency(stored_service&& srv, const std::type_info& info) -> stored_service&
{
	runtime_assert(!find_service_itr(info).valid(), "Service already stored!");
	return deps_.emplace_back(std::move(srv));
}

auto service_impl::add_service_dependency(const stored_service& srv, const std::type_info& info) -> stored_service&
{
	runtime_assert(!find_service_itr(info).valid(), "Service already stored!");
	return deps_.emplace_back(srv);
}

service_state service_impl::state( ) const
{
	return state_;
}

service_impl::load_result service_impl::load(executor& ex) noexcept
{
	switch (state_)
	{
		case service_state::unset:
		{
			const auto mtx = co_await lock_.scoped_lock_async( );
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

			enum class deps_preload_info :uint8_t
			{
				NOTHING
			  , ERROR
			  , SOMETHING
			};

			using std::views::transform;

			const auto have_deps_to_load = [&]
			{
				if (deps_.empty( ))
					return deps_preload_info::NOTHING;

				constexpr auto check_state = [](service_state target)
				{
					return [=](service_state checked)
					{
						return checked == target;
					};
				};

				const auto states = deps_ | transform([](const stored_service& srv) { return srv->state( ); });
				if (std::ranges::any_of(states, check_state(service_state::error)))
					return deps_preload_info::ERROR;
				if (std::ranges::all_of(states, check_state(service_state::loaded)))
					return deps_preload_info::NOTHING;

				return deps_preload_info::SOMETHING;
			};

			const auto load_deps = [&]( )-> load_result
			{
				state_ = service_state::waiting;
				co_await ex.schedule( );
				auto tasks_view      = deps_ | transform([&](const stored_service& srv)-> load_result { return srv->load(ex); });
				const auto&& results = co_await when_all(std::vector(tasks_view.begin( ), tasks_view.end( )));
				co_return std::ranges::all_of(results, [](bool val) { return val == true; });
			};

			bool deps_loaded;
			switch (have_deps_to_load( ))
			{
				case deps_preload_info::NOTHING:
					deps_loaded = true;
					break;
				case deps_preload_info::ERROR:
					deps_loaded = false;
					break;
				case deps_preload_info::SOMETHING:
					deps_loaded = co_await load_deps( );
					break;
				default:
					runtime_assert("Unknown state");
					std::terminate( );
			}

			if (!deps_loaded)
			{
				runtime_assert("Unable to load other services!");
				state_ = service_state::error;
				co_return false;
			}

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
			const auto mtx = co_await lock_.scoped_lock_async( );
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

std::span<const service_impl::stored_service> service_impl::services( ) const
{
	return this->deps_;
}

//----

std::string detail::make_log_message(const service_impl* srv, log_type type, std::string_view extra)
{
	const auto info_msg = [&]
	{
		switch (type)
		{
			case log_type::LOADED: return srv->debug_msg_loaded( );
			case log_type::SKIPPED: return srv->debug_msg_skipped( );
			case log_type::ERROR_: return srv->debug_msg_error( );
			default: throw;
		}
	};

	return std::format("{} \"{}\": {}{}", srv->debug_type( ), srv->name( ), info_msg( ), extra); //todo: colors
}
