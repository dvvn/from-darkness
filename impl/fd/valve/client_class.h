#pragma once

#include <cstdint>

namespace fd
{
struct native_client_class
{
    void *create;
    void *create_event;
    char const *name;
    struct native_recv_table *table;
    native_client_class *next;
    uint32_t id;
};
}