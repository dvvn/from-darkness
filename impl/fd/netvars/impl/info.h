#pragma once

#include "source.h"

// todo: add macro to store only offsets and names

namespace fd
{
class netvar_info final
{
    size_t offset_;
    size_t array_size_;
    netvar_source source_;
    std::string_view name_;

  public:
    /*netvar_info(size_t offset, basic_netvar_type *type, std::string_view name);*/
    netvar_info(size_t offset, uint16_t array_size, netvar_source source, std::string_view name);
    netvar_info(size_t offset, netvar_source source, std::string_view name);

    size_t offset() const;
    std::string_view name() const;
    std::string_view type() const;
    size_t array_size() const;
};

bool operator==(netvar_info const &left, netvar_info const &right);
bool operator==(netvar_info const &left, std::string_view name);
std::strong_ordering operator<=>(netvar_info const &left, netvar_info const &right);
} // namespace fd