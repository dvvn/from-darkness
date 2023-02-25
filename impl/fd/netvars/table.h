#pragma once

#include <fd/netvars/basic_info.h>

#include <boost/ptr_container/ptr_vector.hpp>

namespace fd
{
class netvar_table
{
    std::string name_;
    bool        isRoot_;

    using storage_type   = boost::ptr_vector<basic_netvar_info>;
    using iterator       = storage_type::iterator;
    using const_iterator = storage_type::const_iterator;

    storage_type storage_;

  public:
    ~netvar_table();

    explicit netvar_table(std::string&& name, bool root);
    explicit netvar_table(std::string_view name, bool root);
    explicit netvar_table(const char* name, bool root);
    netvar_table(netvar_table const&) = delete;
    netvar_table(netvar_table&& other) noexcept;

    std::string_view name() const;
    [[deprecated]]
    bool root() const;

    basic_netvar_info const* find(std::string_view name) const;
    void                     add(basic_netvar_info* info);
    void                     sort();

    iterator       begin();
    iterator       end();
    const_iterator begin() const;
    const_iterator end() const;

    bool   empty() const;
    size_t size() const;

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
    netvar_table_multi& inner(std::string&& name);
    netvar_table_multi& inner();
    const netvar_table_multi& inner() const;
}; */
} // namespace fd