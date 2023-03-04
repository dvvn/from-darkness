#include <fd/netvars/type.h>

#include <fmt/format.h>

template <>
struct fmt::formatter<fd::netvar_type_array> : formatter<string_view>
{
    auto format(fd::netvar_type_array const& arr, format_context& ctx) const -> decltype(ctx.out())
    {
        auto inner = arr.data.get();

        memory_buffer tmp;
        auto          it = std::back_inserter(tmp);

        tmp.append(string_view("std::array<"));
        if (std::holds_alternative<fd::netvar_type_array>(inner->data))
            format_to(it, "{}", std::get<fd::netvar_type_array>(inner->data));
        else
            tmp.append(inner->get_type());
        format_to(it, ", {}>", arr.size);

        return formatter<string_view>::format(string_view(tmp.data(), tmp.size()), ctx);
    }
};

namespace fd
{

// netvar_type_array* netvar_type_array::get_inner_array() const
//{
//     auto& inner = this->data->data;
//     return std::holds_alternative<netvar_type_array>(inner) ? &std::get<netvar_type_array>(inner) : nullptr;
// }

netvar_type_array::netvar_type_array(uint16_t size, netvar_type&& type)
    : size(size)
    , data(std::make_unique<netvar_type>(std::move(type)))
{
}

void netvar_type_array::fill(bool force)
{
    if (force || type.empty())
        type = fmt::format("{}", *this);
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
    std::string_view operator()(T& val) const
    {
        return val.type;
    }

    std::string_view operator()(std::monostate) const
    {
        std::unreachable();
    }

    std::string_view operator()(netvar_type_array const& val) const
    {
        const_cast<netvar_type_array&>(val).fill();
        return val.type;
    }
} _GetNetvarType;

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