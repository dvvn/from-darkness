#pragma once

#include <fd/netvars/basic_info.h>
#include <fd/valve/data_map.h>
#include <fd/valve/recv_table.h>

#include <string>
#include <variant>

namespace fd
{
using netvar_info_source = std::variant< //--
    valve::data_map_description const*,
    valve::recv_prop const*>;

class netvar_info final : public basic_netvar_info
{
    size_t             offset_;
    netvar_info_source source_;
    size_t             arraySize_;

    mutable std::string_view name_;
    mutable std::string      type_;

  public:
    netvar_info(size_t offset, netvar_info_source source, std::string_view name, size_t arraySize = 0);

    size_t           offset() const override;
    std::string_view name() const override;
    std::string_view type() const override;

    basic_netvar_info* clone() const override;

    size_t array_size() const;
};

template <typename Fn>
class netvar_info_lazy final : public basic_netvar_info
{
    mutable std::variant<size_t, Fn> getter_;
    std::string_view                 name_;
    std::string                      type_;

  public:
    netvar_info_lazy(Fn getter, const std::string_view name = {}, std::string type = {})
        : getter_(std::move(getter))
        , name_(name)
        , type_(std::move(type))
    {
    }

    size_t offset() const override
    {
        if (std::holds_alternative<Fn>(getter_))
        {
            auto offset = std::get<Fn>(getter_)();
            return getter_.emplace(offset);
        }

        return std::get<size_t>(getter_);
    }

    std::string_view name() const override
    {
        return name_;
    }

    std::string_view type() const override
    {
        return type_;
    }

    basic_netvar_info* clone() const override
    {
        return new netvar_info_lazy(*this);
    }
};

template <typename Fn>
netvar_info_lazy(Fn&&) -> netvar_info_lazy<std::decay_t<Fn>>;

class netvar_info_instant final : public basic_netvar_info
{
    size_t           offset_;
    std::string_view name_;
    std::string      type_;

  public:
    netvar_info_instant(size_t offset, std::string_view name, std::string type);
    netvar_info_instant(size_t offset, std::string_view name, std::string_view type, size_t arraySize);

    size_t           offset() const override;
    std::string_view name() const override;
    std::string_view type() const override;

    basic_netvar_info* clone() const override;
};
} // namespace fd