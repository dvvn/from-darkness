#include "type.h"

#include <fmt/format.h>

namespace fd
{
// netvar_type_array* netvar_type_array::get_inner_array() const
//{
//     auto& inner = this->data->data;
//     return std::holds_alternative<netvar_type_array>(inner) ? &std::get<netvar_type_array>(inner) : nullptr;
// }

static void absolute_array_type(std::string &buff, netvar_type_array &arr)
{
    auto inner = arr.inner.get();

    buff.append("std::array<");
    if (std::holds_alternative<netvar_type_array>(inner->data))
        absolute_array_type(buff, std::get<netvar_type_array>(inner->data));
    else
        buff.append(inner->get_type());

    fmt::format_to(std::back_inserter(buff), ", {}>", arr.size);
}

netvar_type_array::netvar_type_array(uint16_t size, netvar_type &&type)
    : size(size)
    , inner(std::make_unique<netvar_type>(std::move(type)))
{
}

void netvar_type_array::fill(bool force)
{
    if (force || type.empty())
    {
        type.clear();
        absolute_array_type(type, *this);
    }
}

// custom_netvar_type_ex netvar_type_array::unwrap()
//{
//     fill();
//     custom_netvar_type_ex out;
//     out.type = std::move(this->type);
//     get_netvar_include(*this, out.includes);
// }

static constexpr struct
{
    template <class T>
    std::string_view operator()(T &val) const
    {
        return val.type;
    }

    std::string_view operator()(std::monostate) const
    {
        std::unreachable();
    }

    std::string_view operator()(netvar_type_array &val) const
    {
        val.fill();
        return val.type;
    }

    std::string_view operator()(netvar_type_array const &val) const
    {
        assert(!val.type.empty());
        return val.type;
    }
} _GetNetvarType;

std::string_view netvar_type::get_type()
{
    return std::visit(_GetNetvarType, data);
}

std::string_view netvar_type::get_type() const
{
    return std::visit(_GetNetvarType, data);
}
#if 0
bool netvar_type::unwrap(custom_netvar_type& type)
{
    if (!std::holds_alternative<netvar_type_array>(data))
        return false;
    auto& self = std::get<netvar_type_array>(data);
    if (std::holds_alternative<netvar_type_array>(self.data->data))
        return false;

    self.fill();
    type.type = std::move(self.type);
    get_netvar_include(*self.data, type.include);
    return true;
}

bool netvar_type::unwrap(custom_netvar_type_ex& type)
{
    if (!std::holds_alternative<netvar_type_array>(data))
        return false;
    auto& self = std::get<netvar_type_array>(data);

    self.fill();
    type.type = std::move(self.type);
    get_netvar_include(*self.data, type.includes);
    return true;
}
#endif
}