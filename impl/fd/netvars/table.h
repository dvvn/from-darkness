#pragma once

#include <fd/netvars/basic_table.h>

#include <boost/ptr_container/ptr_vector.hpp>

namespace fd
{
class netvar_table final : public basic_netvar_table
{
    std::string name_;

    using storage_type   = boost::ptr_vector<basic_netvar_info>;
    using iterator       = storage_type::iterator;
    using const_iterator = storage_type::const_iterator;
    using pointer        = storage_type::pointer;
    using const_pointer  = basic_netvar_info const*;

    storage_type storage_;

  public:
    ~netvar_table() override;

    explicit netvar_table(std::string&& name);
    explicit netvar_table(std::string_view name);
    explicit netvar_table(char const* name);

    netvar_table(netvar_table const&) = delete;

    netvar_table(netvar_table&& other) noexcept;

    std::string_view name() const override;

    const_pointer find(std::string_view name) const;
    void          add(basic_netvar_info* info);

    void sort();

    bool   empty() const override;
    size_t size() const override;

    
    void       for_each(for_each_fn const& fn) const override;
};

bool operator==(netvar_table const& table, netvar_table const* externalTable);
bool operator==(netvar_table const& table, std::string_view name);

/* class netvar_data_table
{
};

class netvar_table_multi : public netvar_table
{
    std::variant<std::monostate, netvar_table_multi> inner_;

  public:
    bool have_inner() const;
    netvar_table_multi& inner(std::string&& name);
    netvar_table_multi& inner();
    const netvar_table_multi& inner() const;
}; */
} // namespace fd