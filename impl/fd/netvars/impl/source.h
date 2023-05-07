#pragma once

#include "basic_type.h"

#include <cstdint>

namespace fd
{
#if 0
using netvar_source = std::variant< //
    std::monostate,
    valve::data_map_description *,
    valve::recv_prop *>;
#else
namespace valve
{
class recv_prop;
class data_map_description;
} // namespace valve

struct netvar_source
{
    enum class source : uint8_t
    {
        recv_prop,
        data_map
    };

  private:
    void *pointer_;
    source src_;

  public:
    netvar_source(valve::recv_prop *pointer);
    netvar_source(valve::data_map_description *pointer);
    netvar_source(void *pointer, source src);
    char const *name() const;
    size_t offset() const;
    basic_netvar_type *type(std::string_view correct_name, size_t array_size) const;
};

#endif
}