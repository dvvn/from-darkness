#pragma once

#include "basic_type.h"

#include <fd/core.h>

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
struct recv_prop;
struct data_map_description;
} // namespace valve

struct netvar_source1
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
    netvar_source1(valve::recv_prop *pointer);
    netvar_source1(valve::data_map_description *pointer);
    netvar_source1(void *pointer, source src);
    char const * name() const;
    size_t offset() const;
    basic_netvar_type *type(std::string_view correct_name, size_t array_size) const;

    bool operator==(netvar_source1 const & other) const;
};

#endif
}