#pragma once

#include <fd/abstract_interface.h>

namespace fd::valve
{
union entity_list
{
    abstract_interface vtable;
    abstract_function<3, void *, uint32_t> get_client_entity;
};
}