#pragma once

#define CHEAT_CSGO_INTERFACE_INIT(_TYPE_)\
_TYPE_* nstd::one_instance_getter<_TYPE_*>::_Construct( ) const noexcept