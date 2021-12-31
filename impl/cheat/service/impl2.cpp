module;

#include "cheat/console/includes.h"

module cheat.service;
import :loader;
import cheat.console;

using namespace cheat;

// ReSharper disable once CppMemberFunctionMayBeConst
void basic_service::unload( )
{
	auto& loader = services_loader::get( );
	if (this->root_class( ))
		reload_one_instance(loader);
	else
		loader.erase(this->type( ));
}

void basic_service::set_state(service_state state)
{
	runtime_assert(state_ != state);
	runtime_assert(state_ < state);
#ifdef CHEAT_HAVE_CONSOLE
	auto& c = console::get( );
	switch (state)
	{
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

void basic_service_shared::add_to_loader(value_type&& srv) const
{
	const auto loader = services_loader::get_ptr( );
	loader->add_dependency(std::move(srv));
}

// ReSharper disable once CppMemberFunctionMayBeStatic
auto basic_service_shared::get_from_loader(const std::type_info& info) const -> value_type*
{
	const auto loader = services_loader::get_ptr( );
	return loader->find(info);
}