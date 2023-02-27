#pragma once

#include <fd/netvars/basic_table.h>

#include <vector>

namespace fd
{
class netvar_table final : public basic_netvar_table
{
    std::string name_;

    using pointer       = basic_netvar_info*;
    using const_pointer = basic_netvar_info const*;

    std::vector<pointer> storage_;

  public:
    ~netvar_table() override;

    explicit netvar_table(std::string&& name);
    explicit netvar_table(std::string_view name);
    //explicit netvar_table(char const* name);

    netvar_table(netvar_table const& other)            = delete;
    netvar_table& operator=(netvar_table const& other) = delete;

    netvar_table(netvar_table&& other) noexcept;
    netvar_table& operator=(netvar_table&& other) noexcept;

    std::string_view name() const override;

    const_pointer find(std::string_view name) const;
    void          add(pointer info);

    void sort();

    bool   empty() const override;
    size_t size() const override;

    void for_each(for_each_fn const& fn) const override;
};

bool operator==(netvar_table const& table, netvar_table const* externalTable);
bool operator==(netvar_table const& table, std::string_view name);
} // namespace fd
