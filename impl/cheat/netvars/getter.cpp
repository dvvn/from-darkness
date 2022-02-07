module;

#include "includes.h"

module cheat.netvars_getter;
import cheat.netvars;
import cheat.root_service;

size_t cheat::get_netvar_offset(const std::string_view& table, const std::string_view& item)
{
	netvars& ref = services_loader::get( ).deps( ).get<netvars>( );
	return ref.at(table, item);
}