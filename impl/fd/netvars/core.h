#pragma once

#include "tag.h"

namespace fd
{
class library_info;

void store_netvars(void *client_interface);
void store_extra_netvars(void *entity);
void store_custom_netvars(library_info const &client_dll);

void create_netvar_classes(std::wstring_view dir);
void dump_netvars(std::wstring_view dir);

size_t get_netvar_offset(netvar_tag class_name, netvar_tag name);
} // namespace fd