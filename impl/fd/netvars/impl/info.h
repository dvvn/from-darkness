#pragma once

#include "hashed_object.h"
#include "source.h"

// todo: add macro to store only offsets and names

namespace fd
{
template <typename T>
hashed_object(T const *) -> hashed_object<std::basic_string_view<T>>;

class netvar_info final
{
    using hashed_name = hashed_object<std::string_view>;

    size_t offset_;
    size_t array_size_;
    netvar_source1 source_;
    hashed_name name_;

  public:
    netvar_info(size_t offset, uint16_t array_size, netvar_source1 source, hashed_name const &name);
    netvar_info(size_t offset, netvar_source1 source, hashed_name const &name);

    size_t offset() const;
    std::string_view name() const;
    std::string_view type() const;
    size_t array_size() const;

    bool operator==(netvar_info const &other) const;
    bool operator==(std::string_view name) const;
    bool operator==(hashed_name const &name_hash) const;
    bool operator==(size_t name_hash) const;
};
} // namespace fd