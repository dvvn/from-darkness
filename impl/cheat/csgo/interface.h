#pragma once

#include <utility>

import nstd.one_instance;

#define CHEAT_CSGO_INTERFACE_INIT(_TYPE_,_IMPL_)\
template<>\
template<>\
nstd::one_instance_getter<_TYPE_*>::one_instance_getter(const std::in_place_index_t<0>)\
	: item_(_IMPL_)\
{\
}