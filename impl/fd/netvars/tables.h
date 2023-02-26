#pragma once

#include <fd/netvars/table.h>

#define FD_NETVARS_DT_MERGE

namespace fd
{
struct netvar_tables
{
    using storage_type = std::vector<netvar_table>;

    using pointer       = storage_type::pointer;
    using const_pointer = storage_type::const_pointer;

    using iterator = storage_type::const_iterator;

  private:
    storage_type storage_;

  public:
    pointer add(netvar_table&& table);
    pointer add(std::string&& name);
    pointer add(std::string_view name);

    pointer       find(std::string_view name);
    const_pointer find(std::string_view name) const;

    size_t index_of(const_pointer table) const;
    void   sort(size_t index);

    size_t size() const;
    bool   empty() const;

#if 0
    iterator      begin() const;
    iterator      end() const;
    const_pointer data() const;
#endif

    using updater_fn = std::function<bool(basic_netvar_table const*&)>;
    updater_fn make_updater() const;
};

class netvar_tables_ordered : public netvar_tables
{
    std::vector<size_t> sortReqests_;

    using netvar_tables::sort;

  public:
    netvar_tables_ordered();

    void request_sort(const_pointer table);
    void sort();
};

} // namespace fd
