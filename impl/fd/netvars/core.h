#pragma once

#include <fd/core.h>

#include <string_view>

namespace fd
{

void store_netvars(void *client_interface);
void store_extra_netvars(void *entity);
void store_custom_netvars(class library_info client_dll);

void create_netvar_classes(std::wstring_view dir);
void dump_netvars(std::wstring_view dir);

size_t get_netvar_offset(std::string_view class_name, std::string_view name);
size_t get_netvar_offset(size_t class_name, size_t name);
} // namespace fd