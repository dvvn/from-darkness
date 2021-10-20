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

size_t service_base::_Services_count( )
{
	return nstd::one_instance<services_counter>::get_ptr( )->count;
}

service_base::service_base( )
{
	++nstd::one_instance<services_counter>::get_ptr( )->count;
}

service_base::~service_base( )
{
	_Loading_access_assert(state_);
	--nstd::one_instance<services_counter>::get_ptr( )->count;
}

service_base::service_base(service_base&& other) noexcept
{
	*this = std::move(other);
	++nstd::one_instance<services_counter>::get_ptr( )->count;
}

service_base& service_base::operator=(service_base&& other) noexcept
{
	_Loading_access_assert(this->state_);
	_Loading_access_assert(other.state_);

	std::swap(state_, other.state_);
	std::swap(deps_, other.deps_);

	return *this;
}

service_base::stored_service service_base::find_service(const std::type_info& info) const
{
	for (const auto& service: this->deps_)
	{
		if (service->type( ) == info)
			return service;
	}

	return {};
}

service_base::stored_service* service_base::find_service_ptr(const std::type_info& info)
{
	for (auto& service: this->deps_)
	{
		if (service->type( ) == info)
			return std::addressof(service);
	}

	return (nullptr);
}

struct dummy_service final : service_info<dummy_service>
{
protected:
	load_result load_impl( ) noexcept override
	{
		CHEAT_SERVICE_SKIPPED
	}
};

void service_base::remove_service(const std::type_info& info)
{
	const auto ptr = find_service_ptr(info);
	if (ptr == nullptr)
		return;
	*ptr = std::make_shared<dummy_service>( );
}

void service_base::add_service_dependency(stored_service&& srv, const std::type_info& info)
{
	runtime_assert(find_service(info) == stored_service( ), "Service already stored!");
	this->deps_.push_back(std::move(srv));
}

service_state service_base::state( ) const
{
	return state_;
}

service_base::load_result service_base::load(executor& ex) noexcept
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

std::span<const service_base::stored_service> service_base::services( ) const
{
	return this->deps_;
}

//----

std::string detail::make_log_message(const service_base* srv, log_type type, std::string_view extra)
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
