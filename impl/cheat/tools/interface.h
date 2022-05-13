#pragma once

#include <utility>

import nstd.one_instance;

#define CHEAT_OBJECT_IMPL(_TYPE_,_IMPL_)\
template<>\
template<>\
nstd::one_instance_getter<_TYPE_>::one_instance_getter(const std::in_place_index_t<0>)\
	: item_(_IMPL_)\
{\
}

#define CHEAT_INTERFACE_IMPL(_TYPE_,_IMPL_) CHEAT_OBJECT_IMPL(_TYPE_*,_IMPL_)

namespace cheat
{
    using nstd::instance_of;
    using nstd::one_instance;
}