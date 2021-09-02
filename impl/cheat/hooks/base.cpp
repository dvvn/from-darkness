#include "base.h"

#include "cheat/core/console.h"

using cheat::hooks::detail::helper;
using cheat::service_base;

service_base::load_result helper::load_impl( )
{
	co_return this->hook( ) && this->enable( ) ? service_state::loaded : service_state::error;
}

std::string_view helper::object_name( ) const
{
	return "hook";
}

void helper::reset( )
{
	this->disable_after_call( );
	service_base::reset( );
}


