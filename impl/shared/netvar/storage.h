#pragma once

#include "basic_storage.h"

namespace fd
{
template <class T>
struct make_incomplete_object;

class netvar_storage;

template <>
struct make_incomplete_object<netvar_storage> final
{
    basic_netvar_storage* operator()() const;
};
}