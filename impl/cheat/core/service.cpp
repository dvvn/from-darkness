#include "service.h"

#include "console.h"
#include "csgo interfaces.h"

#include <cppcoro/static_thread_pool.hpp>
#include <cppcoro/async_mutex.hpp>

using namespace cheat;

struct service_base_fields::hidden_type
{
	//cppcoro::async_mutex                      lock;
	std::vector<service_base::stored_service> deps; //dependencies what must be loaded before
};

template <typename T>
static void _Loading_access_assert([[maybe_unused]] T&& state)
{
	runtime_assert(state != service_state::loading, "Unable to modify service while loading!");
}

service_base_fields::service_base_fields( )
{
	hidden = std::make_unique<hidden_type>( );
	state  = service_state::unset;
}

service_base_fields::~service_base_fields( )
{
	_Loading_access_assert(state);
}

service_base_fields::service_base_fields(service_base_fields&& other) noexcept
{
	_Loading_access_assert(other.state);

	state  = static_cast<service_state>(other.state);
	hidden = std::move(other.hidden);
}

service_base_fields& service_base_fields::operator=(service_base_fields&& other) noexcept
{
	_Loading_access_assert(this->state);
	_Loading_access_assert(other.state);

	//std::swap(state, other.state);
	const auto state_old = static_cast<service_state>(state);
	state                = static_cast<service_state>(other.state);
	other.state          = state_old;

	std::swap(hidden, other.hidden);

	return *this;
}

std::string_view service_base::object_name( ) const
{
	return "service";
}

service_base::stored_service service_base::find_service(const std::type_info& info) const
{
	for (const auto& service: fields( ).hidden->deps)
	{
		if (service->type( ) == info)
			return service;
	}

	return { };
}

void service_base::add_service_dependency(stored_service&& srv, const std::type_info& info)
{
	runtime_assert(find_service(info) == stored_service( ), "Service already stored!");
	fields( ).hidden->deps.push_back(std::move(srv));
}

service_state service_base::state( ) const
{
	return fields( ).state;
}

void service_base::reset( )
{
	auto& f = this->fields( );
	_Loading_access_assert(f.state);
	f.hidden->deps.clear( );
	f.state = service_state::unset;
}

bool service_base::validate_init_state( ) const
{
	if (fields( ).state == service_state::unset)
		return true;

	runtime_assert("Unable to validate state!");
	return false;
}

static void _Service_msg([[maybe_unused]] const service_base* owner, [[maybe_unused]] const std::string_view& msg)
{
	CHEAT_CONSOLE_LOG(std::ostringstream( )
					  << owner->name( ) << ' '
					  << owner->object_name( ) << ": "
					  << msg);
}

service_base::load_result service_base::load(executor& ex)
{
	auto& f = this->fields( );

	/*const auto lock =std::scoped_lock*/
	//co_await(f.hidden->lock.lock_async( ));

	if (!this->validate_init_state( ))
		co_return f.state;

	f.state = service_state::waiting;
	if (!co_await this->wait_for_others(ex))
	{
		_Service_msg(this, "error while child waiting");
		co_return f.state = service_state::error;
	}

	f.state = service_state::loading;
	co_await ex.schedule( );
	switch ((co_await this->load_impl( )))
	{
		case service_state::loaded:
		{
			this->after_load( );
			co_return f.state = service_state::loaded;
		}
		case service_state::skipped:
		{
			this->after_skip( );
			co_return f.state = service_state::skipped;
		}
		default:
		{
			this->after_error( );
			co_return f.state = service_state::error;
		}
	}
}

service_base::child_wait_result service_base::wait_for_others(executor& ex)
{
	auto& deps = fields( ).hidden->deps;

	for (auto itr = deps.begin( ); itr != deps.end( );)
	{
		const auto& se = *itr;
		switch (se->state( ))
		{
			case service_state::unset:
				co_await ex.schedule( );
				co_await se->load(ex);
				break;
			case service_state::loaded:
			case service_state::skipped:
				break;
			case service_state::error:
				co_return false;
			case service_state::waiting:
			case service_state::loading:
				runtime_assert("Service still loading!");
				break;
			default:
				runtime_assert("Update function!");
				break;
		}

		++itr;
	}

	co_return true;
}

void service_base::after_load( )
{
	_Service_msg(this, "loaded");
}

void service_base::after_skip( )
{
	_Service_msg(this, "skipped");
}

void service_base::after_error( )
{
	_Service_msg(this, "error while loading");
}

std::span<const service_base::stored_service> service_base::services( ) const
{
	return fields( ).hidden->deps;
}

service_maybe_skipped::service_maybe_skipped(bool skip)
	: skipped_(skip)
{
}

service_base::load_result service_maybe_skipped::load(executor& ex)
{
	auto& f = this->fields( );

	/*const auto lock =std::scoped_lock*/
	//co_await(f.hidden->lock.lock_async( ));

	if (!skipped_)
		co_return co_await service_base::load(ex);

	if (this->validate_init_state( ))
	{
		f.state = service_state::skipped;
		this->after_skip( );
	}
	co_return f.state;
}

bool service_maybe_skipped::always_skipped( ) const
{
	return skipped_;
}
