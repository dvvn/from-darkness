#pragma once
#include <fd/abstract_interface.h>

namespace fd::valve
{
struct client_class
{
    void *create;
    void *create_event;
    char const *name;
    struct recv_table *table;
    client_class *next;
    uint32_t id;
};

union client
{
    FD_ABSTRACT_INTERFACE(client);
    abstract_function<8, client_class *> get_all_classes;
};
}