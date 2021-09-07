#include "service.h"

#include "console.h"

#include <cppcoro/static_thread_pool.hpp>

using namespace cheat;

struct service_base_fields::storage_type: std::vector<service_base::stored_service>
{
};

template <typename T>
static void _Loading_access_assert([[maybe_unused]] T&& state)
{
	runtime_assert(state != service_state::loading, "Unable to modify service while loading!");
}

service_base_fields::service_base_fields( )
{
	deps  = std::make_unique<storage_type>( );
	state = service_state::unset;
}

service_base_fields::~service_base_fields( )
{
	_Loading_access_assert(state);
}

service_base_fields::service_base_fields(service_base_fields&& other) noexcept
{
	_Loading_access_assert(other.state);

	state = other.state;
	deps  = std::move(other.deps);
}

service_base_fields& service_base_fields::operator=(service_base_fields&& other) noexcept
{
	_Loading_access_assert(this->state);
	_Loading_access_assert(other.state);

	std::swap(state, other.state);
	std::swap(deps, other.deps);

	return *this;
}

std::string_view service_base::object_name( ) const
{
	return "service";
}

service_base::stored_service service_base::find_service(const std::type_info& info) const
{
	for (const auto& service: *fields( ).deps)
	{
		if (service->type( ) == info)
			return service;
	}

	return { };
}

void service_base::store_service(stored_service&& srv, const std::type_info& info)
{
	(void)this;
	runtime_assert(find_service(info) == stored_service( ), "Service already stored!");
	fields( ).deps->push_back(std::move(srv));
}

service_state service_base::state( ) const
{
	return fields( ).state;
}

void service_base::reset( )
{
	_Loading_access_assert(fields( ).state);
	fields( ).deps->clear( );
	fields( ).state = { };
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

	if (!this->validate_init_state( ))
		co_return f.state;

	f.state = service_state::waiting;
	if (!co_await this->wait_for_others(ex))
	{
		_Service_msg(this, "error while child waiting");
		co_return f.state = service_state::error;
	}
	co_await ex.schedule( );
	f.state = service_state::loading;
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
	(void)this;
	for (auto itr = fields( ).deps->begin( ); itr != fields( ).deps->end( );)
	{
		const auto& se = *itr;
		switch (se->state( ))
		{
			case service_state::unset:
				co_await ex.schedule( );
				co_await se->load(ex);
				continue;
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
	return *fields( ).deps;
}

service_sometimes_skipped::service_sometimes_skipped(bool skip)
	: skipped_(skip)
{
}

service_base::load_result service_sometimes_skipped::load(executor& ex)
{
	if (!skipped_)
		co_return co_await service_base::load(ex);

	auto& f = this->fields( );
	if (this->validate_init_state( ))
	{
		f.state = service_state::skipped;
		this->after_skip( );
	}
	co_return f.state;
}

bool service_sometimes_skipped::always_skipped( ) const
{
	return skipped_;
}
