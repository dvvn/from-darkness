#pragma once

#include "source.h"

// todo: add macro to store only offsets and names

namespace fd
{
struct hashed_netvar_name : std::pair<size_t, std::string_view>
{
    hashed_netvar_name(std::string_view name);
    hashed_netvar_name(const char *name);

    bool operator==(hashed_netvar_name const &other) const;
    bool operator==(std::string_view name) const;

    std::string_view const *operator->() const;
    std::string_view get() const;
};

class netvar_info final
{
    size_t offset_;
    size_t array_size_;
    netvar_source source_;
    hashed_netvar_name name_;

  public:
    netvar_info(size_t offset, uint16_t array_size, netvar_source source, hashed_netvar_name const &name);
    netvar_info(size_t offset, netvar_source source, hashed_netvar_name const &name);

    size_t offset() const;
    std::string_view name() const;
    std::string_view type() const;
    size_t array_size() const;

    bool operator==(netvar_info const &other) const;
    bool operator==(std::string_view name) const;
    bool operator==(hashed_netvar_name const &name_hash) const;
    bool operator==(size_t name_hash) const;
};
} // namespace fd