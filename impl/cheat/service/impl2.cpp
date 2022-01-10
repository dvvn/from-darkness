module;

#include "cheat/console/includes.h"

module cheat.service;
import :loader;
import cheat.console;

using namespace cheat;

void basic_service::unload( )
{
	auto loader = services_loader::get_ptr( );
	auto &this_type = this->type( );
	if (this_type == loader->type( ))
	{
		this->deps_.clear( );
		this->state_ = service_state::unset;
	}
	else
	{
		//todo: erase & updater indexes
		auto sptr = loader->find(this_type);
		auto ptr = sptr->get( );
		ptr->deps_.clear( );
		ptr->set_state(service_state::unset);
	}
}

void basic_service::set_state(service_state state)
{
	runtime_assert(state_ != state);

#ifdef CHEAT_HAVE_CONSOLE
	auto& c = services_loader::get( ).get_dependency<console>( );
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

#if 0
void basic_service_shared::add_to_loader(value_type&& srv) const
{
	const auto loader = services_loader::get_ptr( );
	loader->add_dependency(std::move(srv));
}

auto basic_service_shared::get_from_loader(const std::type_info& info) const -> value_type*
{
	const auto loader = services_loader::get_ptr( );
	return loader->find(info);
}
#endif

basic_service* cheat::get_root_service( )
{
	return services_loader::get_ptr( );
}