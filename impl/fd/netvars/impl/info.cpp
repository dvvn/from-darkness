#include "info.h"

namespace fd
{
netvar_info::netvar_info(size_t offset, uint16_t array_size, netvar_source source, _const<hashed_name &> name)
    : offset_(offset)
    , array_size_(array_size)
    , source_(source)
    , name_((name))

{
}

netvar_info::netvar_info(size_t offset, netvar_source source, _const<hashed_name &> name)
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

bool netvar_info::operator==(_const<netvar_info &> other) const
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

bool netvar_info::operator==(_const<hashed_name &> name_hash) const
{
    return name_ == name_hash;
}

bool netvar_info::operator==(size_t name_hash) const
{
    return std::get<size_t>(name_) == name_hash;
}
}