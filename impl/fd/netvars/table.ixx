module;

#include <fd/utility.h>

#include <concepts>
#include <vector>

export module fd.netvars.table;
export import fd.netvars.info;
export import fd.memory;

using namespace fd;

using hashed_string_view = string_view;
using hashed_string      = string;

template <class T>
auto make_deleter(T*)
{
    return [](void* vptr) {
        constexpr default_delete<T> del;
        del(static_cast<T*>(vptr));
    };
}

template <class T>
using unique_ptr_ex = unique_ptr<T, void (*)(void*)>;

class netvar_table : public std::vector<unique_ptr_ex<basic_netvar_info>>
{
    hashed_string name_;
    bool root_;

  protected:
    void validate_item(const basic_netvar_info* info) const;

  public:
    netvar_table(hashed_string&& name, const bool root);

    hashed_string_view name() const;
    bool root() const;
    const basic_netvar_info* find(const hashed_string_view name) const;

    template <typename... Args>
    const auto* add(Args&&... args)
    {
        auto ptr = make_netvar_info(std::forward<Args>(args)...);
#ifdef _DEBUG
        validate_item(ptr);
#endif
        this->emplace_back(ptr, make_deleter(ptr));
        return ptr;
    }
};

/* class netvar_data_table
{
};

class netvar_table_multi : public netvar_table
{
    std::variant<std::monostate, netvar_table_multi> inner_;

  public:
    bool have_inner() const;
    netvar_table_multi& inner(hashed_string&& name);
    netvar_table_multi& inner();
    const netvar_table_multi& inner() const;
}; */

export namespace fd
{
    using ::netvar_table;
} // namespace fd
