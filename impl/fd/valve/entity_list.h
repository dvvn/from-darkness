#pragma once

#include <fd/abstract_interface.h>

namespace fd::valve
{
// ReSharper disable CppInconsistentNaming
class CClientEntityList;
class IClientEntityList;
// ReSharper restore CppInconsistentNaming

union entity_list
{
    FD_ABSTRACT_INTERFACE(IClientEntityList);
    abstract_function<3, void *, named<uint32_t, "index">> get_client_entity;
    abstract_function<8, int32_t> max_entities;
};
}