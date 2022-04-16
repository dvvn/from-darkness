#pragma once

import nstd.one_instance;

#define CHEAT_CSGO_INTERFACE_INIT(_TYPE_)\
_TYPE_* nstd::one_instance_getter<_TYPE_*>::_Construct( ) const noexcept

//namespace cheat::csgo
//{
//	template<typename T>
//	T get_interface( ) noexcept;
//}
//
//#define CHEAT_CSGO_INTERFACE_INIT(_TYPE_) _TYPE_* get_interface<_TYPE_*>( ) noexcept