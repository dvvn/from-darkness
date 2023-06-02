#include "hash.h"
#include "info.h"

#include <cassert>

namespace fd
{

hashed_netvar_name::hashed_netvar_name(std::string_view name)
    : std::pair<size_t, std::string_view>(netvar_hash(name), name)
{
    assert(!name.empty());
}

hashed_netvar_name::hashed_netvar_name(const char *name)
    : hashed_netvar_name(std::string_view(name))
{
}

bool hashed_netvar_name::operator==(hashed_netvar_name const &other) const
{
    return std::get<size_t>(*this) == std::get<size_t>(other);
}

bool hashed_netvar_name::operator==(std::string_view other) const
{
    return std::get<std::string_view>(*this) == other;
}

std::string_view const *hashed_netvar_name::operator->() const
{
    return &std::get<std::string_view>(*this);
}

std::string_view hashed_netvar_name::get() const
{
    return std::get<std::string_view>(*this);
}

netvar_info::netvar_info(size_t offset, uint16_t array_size, netvar_source source, hashed_netvar_name const &name)
    : offset_(offset)
    , array_size_(array_size)
    , source_(source)
    , name_((name))

{
}

netvar_info::netvar_info(size_t offset, netvar_source source, hashed_netvar_name const &name)
    : offset_(offset)
    , array_size_(0)
    , source_(source)
    , name_((name))

{
}

size_t netvar_info::offset() const
{
    return offset_;
}

std::string_view netvar_info::name() const
{
    return std::get<std::string_view>(name_);
}

std::string_view netvar_info::type() const
{
    return source_.type(name(), array_size_)->get();
}

size_t netvar_info::array_size() const
{
    /*if (std::holds_alternative<netvar_type_array>(type_.data))
        return std::get<netvar_type_array>(type_.data).size;*/
    return netvar_type_array_size(source_.type(name(), array_size_));
}

bool netvar_info::operator==(netvar_info const &other) const
{
    return name_ == other.name_ &&             //
           offset_ == other.offset_ &&         //
           array_size_ == other.array_size_ && //
           source_ == other.source_;
}

bool netvar_info::operator==(std::string_view name) const
{
    return std::get<std::string_view>(name_) == name;
}

bool netvar_info::operator==(hashed_netvar_name const &name_hash) const
{
    return name_ == name_hash;
}

bool netvar_info::operator==(size_t name_hash) const
{
    return std::get<size_t>(name_) == name_hash;
}
}