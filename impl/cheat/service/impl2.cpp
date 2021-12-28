module;

#include <nstd/one_instance.h>

module cheat.core.service;
import cheat.core.services_loader;

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