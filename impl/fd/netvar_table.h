#pragma once

#include <fd/netvar_info.h>

#include <vector>

namespace fd
{
class netvar_table
{
    string name_;
    bool   isRoot_;

    std::vector<basic_netvar_info*> storage_;

  public:
    ~netvar_table();

    netvar_table(string&& name, bool root);
    netvar_table(const netvar_table&) = delete;
    netvar_table(netvar_table&& other) noexcept;

    string_view name() const;
    bool        root() const;

    const basic_netvar_info* find(string_view name) const;
    void                     add(basic_netvar_info* info);
    void                     sort();

    basic_netvar_info**       begin();
    basic_netvar_info**       end();
    basic_netvar_info* const* begin() const;
    basic_netvar_info* const* end() const;

#if 0
    template <typename... Args>
    const auto* add(Args&&... args)
    {
        auto ptr = make_netvar_info(std::forward<Args>(args)...);
#ifdef _DEBUG
        validate_item(ptr);
#endif
        this->emplace_back(ptr /* , make_deleter(ptr) */);
        return ptr;
    }
#endif
};

/* class netvar_data_table
{
};

class netvar_table_multi : public netvar_table
{
    std::variant<std::monostate, netvar_table_multi> inner_;

  public:
    bool have_inner() const;
    netvar_table_multi& inner(string&& name);
    netvar_table_multi& inner();
    const netvar_table_multi& inner() const;
}; */
} // namespace fd