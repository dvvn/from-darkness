#pragma once

#include <fd/abstract_interface.h>

namespace fd::valve
{
union entity_list
{
    FD_ABSTRACT_INTERFACE(entity_list);
    abstract_function<3, void *, named<uint32_t, "index">> get_client_entity;
};
}