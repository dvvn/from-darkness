#pragma once

#include <fd/core.h>

namespace fd
{
struct wstring_view;
struct string_view;

void store_netvars(void *client_interface);
void store_extra_netvars(void *entity);
void store_custom_netvars(struct valve_library client_dll);

void create_netvar_classes(wstring_view dir);
void dump_netvars(wstring_view dir);

size_t get_netvar_offset(string_view class_name, string_view name);
size_t get_netvar_offset(size_t class_name, size_t name);
} // namespace fd