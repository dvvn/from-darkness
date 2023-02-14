#pragma once

#include <fd/functional.h>
#include <fd/string.h>
#include <fd/valve/data_map.h>
#include <fd/valve/recv_table.h>

#include <variant>

namespace fd
{
struct basic_netvar_info
{
    virtual ~basic_netvar_info() = default;

    virtual size_t      offset() const = 0;
    virtual string_view name() const   = 0;
    virtual string_view type() const   = 0;
};

using netvar_info_source = std::variant< //--
    const valve::data_map_description*,
    const valve::recv_prop*>;

class netvar_info final : public basic_netvar_info
{
    size_t             offset_;
    netvar_info_source source_;
    size_t             arraySize_; // for arrays

    mutable string_view name_;
    mutable string      type_;

  public:
    netvar_info(size_t offset, netvar_info_source source, size_t arraySize = 0, string_view name = {});

    size_t      offset() const override;
    string_view name() const override;
    string_view type() const override;

    size_t array_size() const;
};

template <typename Fn>
class netvar_info_lazy final : public basic_netvar_info
{
    mutable std::variant<size_t, Fn> getter_;
    string_view                      name_;
    string                           type_;

  public:
    netvar_info_lazy(Fn getter, const string_view name = {}, string type = {})
        : getter_(std::move(getter))
        , name_(name)
        , type_(std::move(type))
    {
    }

    size_t offset() const override
    {
        if (std::holds_alternative<Fn>(getter_))
        {
            const auto offset = std::get<Fn>(getter_)();
            return getter_.emplace(offset);
        }

        return std::get<size_t>(getter_);
    }

    string_view name() const override
    {
        return name_;
    }

    string_view type() const override
    {
        return type_;
    }
};

template <typename Fn>
netvar_info_lazy(Fn&&) -> netvar_info_lazy<std::decay_t<Fn>>;

class netvar_info_instant final : public basic_netvar_info
{
    size_t      offset_;
    string_view name_;
    string      type_;

  public:
    netvar_info_instant(size_t offset, string_view name = {}, string&& type = {});

    size_t      offset() const override;
    string_view name() const override;
    string_view type() const override;
};
} // namespace fd