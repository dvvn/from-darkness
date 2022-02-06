module;

#include "cheat/console/includes.h"

module cheat.service:impl;
import :root;
import cheat.console;

using namespace cheat;

static void _Print_state(basic_service* holder, service_state state)
{
	console* c = nullptr;
	if (holder->type( ) == typeid(console))
	{
		c = dynamic_cast<console*>(holder);
	}
	else
	{
		constexpr auto find_console = [](basic_service* from)->console*
		{
			for (auto& d : from->_Deps<false>( ))
			{
				if (d->type( ) != typeid(console))
					continue;
				return dynamic_cast<console*>(d.get( ));

			}
			return nullptr;
		};

		c = find_console(holder);
		if (!c)
		{
#ifdef _DEBUG
			c = find_console(services_loader::get_ptr( ));
			if (!c)
#endif
				return;
		}
	}

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

void basic_service::set_state(service_state state)
{
	runtime_assert(state_ != state);
	_Print_state(this, state);
	state_ = state;
}
