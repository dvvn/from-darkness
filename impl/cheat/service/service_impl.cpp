module;

#include "cheat/console/includes.h"

module cheat.service:impl;
import :root;
import cheat.console;

using namespace cheat;

static void _Print_state_impl(console* c, basic_service* holder, service_state state)
{
	switch (state)
	{
	case service_state::unset:
	{
		c->log("{} - Unloaded.", holder->name( ));
		break;
	}
	case service_state::waiting:
	{
		auto deps = holder->_Deps<false>( );
		c->log("{} - Trying to load {} dependenc{}...", holder->name( ), deps.size( ), deps.size( ) == 1 ? "y" : "ies");
		break;
	}
	case service_state::loading:
	{
		c->log("{} - Loading started.", holder->name( ));
		break;
	}
	case service_state::loaded:
	{
		c->log("{} - Loaded successfully.", holder->name( ));
		break;
	}
	case service_state::error:
	{
		c->log("{} - Not loaded.", holder->name( ));
		break;
	}
	/*default:
		if (!extra_text.empty( ))
			c->log("{} - {}", holder->name()( ),extra_text);
		break;*/
	}
}

static void _Print_state(basic_service* holder, service_state state)
{
#ifdef _DEBUG
	if (services_loader::get( ).deps( ).try_call<console>(_Print_state_impl, holder, state).called)
		return;
#endif
	if (holder->type( ) == typeid(console))
	{
		_Print_state_impl(dynamic_cast<console*>(holder), holder, state);
		return;
	}
	for (auto& d : holder->_Deps<false>( ))
	{
		if (d->type( ) != typeid(console))
			continue;
		_Print_state_impl(dynamic_cast<console*>(d.get( )), holder, state);
		return;
	}
}

void basic_service::set_state(service_state state)
{
	runtime_assert(state_ != state);
	_Print_state(this, state);
	state_ = state;
}
