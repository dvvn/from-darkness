#include "base.h"

#include "cheat/core/console.h"

using namespace cheat;

service_base::load_result hooks::helper::load_impl( )
{
	co_return this->hook( ) && this->enable( ) ? service_state::loaded : service_state::error;
}

std::string_view hooks::helper::object_name( ) const
{
	return "hook";
}

void hooks::helper::reset( )
{
	this->disable( );
	service_base::reset( );
}
