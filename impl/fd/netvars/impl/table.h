#pragma once

#include "info.h"

#include <boost/core/noncopyable.hpp>

#include <vector>

namespace fd
{

struct netvar_table final : std::vector<netvar_info>, boost::noncopyable
{
    using string_type      = std::string;
    using hashed_name      = hashed_object<string_type>;
    using hashed_name_view = hashed_object<std::string_view>;

  private:
    hashed_name name_;

  public:
    netvar_table() = default;

    template <typename T>
    netvar_table(T &&name) requires(std::constructible_from<hashed_name, T>)
        : name_(std::forward<T>(name))
    {
    }

    netvar_table(netvar_table &&other) noexcept;
    netvar_table &operator=(netvar_table &&other) noexcept;

    std::string_view name() const;
    size_t name_hash() const;

    void sort();
    void on_item_added(std::string_view name) const;

    bool operator==(_const<hashed_name_view &> name_hash) const;
    bool operator==(std::string_view name) const;
    bool operator==(size_t name_hash) const;
};

} // namespace fd