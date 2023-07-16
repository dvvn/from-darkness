#pragma once

#include "basic_netvar_storage.h"
#include "interface_holder.h"

#define FD_MERGE_NETVAR_TABLES

namespace fd
{
struct string;

class basic_netvar_type_cache
{
  protected:
    ~basic_netvar_type_cache() = default;

  public:
    using key_type = void const *;

    virtual string_view get(key_type key) const             = 0;
    virtual string_view store(key_type key, string &&value) = 0;
};

class netvar_storage;
FD_INTERFACE_FWD(netvar_storage,  basic_netvar_storage);
}