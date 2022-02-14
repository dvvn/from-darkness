module;

#include "cheat/console/includes.h"

module cheat.service:tools;
import cheat.console;
import cheat.root_service;

using cheat::console;
using cheat::service_state;
using cheat::basic_service;

static void _Log(console* c, basic_service* holder, service_state state)
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

void cheat::log_service_state(basic_service* holder, service_state state)
{
	const auto c = services_loader::get( ).deps( ).try_get<console>( );
	if (c)
		return _Log(c, holder, state);

	if (holder->type( ) == typeid(console))
		return _Log(dynamic_cast<console*>(holder), holder, state);

	for (auto& d : holder->_Deps<false>( ))
	{
		if (d->type( ) != typeid(console))
			continue;
		_Log(dynamic_cast<console*>(d.get( )), holder, state);
		break;
	}
}