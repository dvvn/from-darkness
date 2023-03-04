#pragma once

#include <fd/netvars/info.h>

#include <vector>

namespace fd
{
class netvar_table final : public std::vector<netvar_info>
{
    std::string name_;

  public:
    netvar_table() = default;

    explicit netvar_table(std::string&& name);
    explicit netvar_table(std::string_view name);
    // explicit netvar_table(char const* name);

    netvar_table(netvar_table const& other)            = delete;
    netvar_table& operator=(netvar_table const& other) = delete;

    netvar_table(netvar_table&& other) noexcept;
    netvar_table& operator=(netvar_table&& other) noexcept;

    void set_name(std::string&& name);

    std::string_view name() const;
    const_pointer    find(std::string_view name) const;
    void             sort();
    void             on_item_added(std::string_view name) const;
};

bool operator==(netvar_table const& table, std::string_view name);
} // namespace fd