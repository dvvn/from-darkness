module;

#include "cheat/console/includes.h"

module cheat.service:impl;
import :root;
import cheat.console;

using namespace cheat;

basic_service* cheat::get_root_service( )
{
	return services_loader::get_ptr( );
}

void basic_service::set_state(service_state state)
{
	runtime_assert(state_ != state);

#ifdef CHEAT_HAVE_CONSOLE
	auto& c = services_loader::get( ).deps( ).get<console>( );
	switch (state)
	{
	case service_state::unset:
	{
		c.log("{} - Unloaded.", name( ));
		break;
	}
	case service_state::waiting:
		c.log("{} - Trying to load {} dependencies...", name( ), deps_.size( ));
		break;
	case service_state::loading:
		c.log("{} - Loading started.", name( ));
		break;
	case service_state::loaded:
		c.log("{} - Loaded successfully.", name( ));
		break;
	case service_state::error:
		c.log("{} - Not loaded.", name( ));
		break;
		/*default:
			if (!extra_text.empty( ))
				c.log("{} - {}", name( ),extra_text);
			break;*/
	}
#endif	
	state_ = state;
}
