﻿module;

#include <functional>

module cheat.csgo.interfaces.BaseClient;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

IBaseClientDLL* nstd::one_instance_getter<IBaseClientDLL*>::_Construct( )const
{
	return csgo_modules::client.find_interface<"VClient">( );
}

bool IBaseClientDLL::DispatchUserMessage(int msg_type, int flags, int size, const void* msg)
{
	//return dhooks::invoke(&IBaseClientDLL::DispatchUserMessage, static_cast<size_t>(38), this, msg_type, flags, size, msg);
	const auto fn = nstd::basic_address(this).deref<1>( )[38].get<decltype(&IBaseClientDLL::DispatchUserMessage)>( );
	return std::invoke(fn, this, msg_type, flags, size, msg);
}
