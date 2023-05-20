#pragma once

#include "info.h"

#include <boost/core/noncopyable.hpp>

#include <vector>

namespace fd
{
struct hashed_netvar_table_name : std::pair<size_t, std::string>
{
    hashed_netvar_table_name();

    hashed_netvar_table_name(std::string_view name);
    hashed_netvar_table_name(std::string &&name);
    hashed_netvar_table_name(const char *name);
    hashed_netvar_table_name(hashed_netvar_name const &name);
    

    bool operator==(hashed_netvar_table_name const &other) const;
    bool operator==(std::string_view other) const;

    std::string const *operator->() const;
    std::string_view get() const;
};

class netvar_table final : public std::vector<netvar_info>, public boost::noncopyable
{
#if 0
    hashed_netvar_name name_
#else
    hashed_netvar_table_name name_;
#endif

  public:
    netvar_table();
    netvar_table(hashed_netvar_table_name &&name);
    netvar_table(netvar_table &&other) noexcept;

    netvar_table &operator=(netvar_table &&other) noexcept;

    std::string_view name() const;
    size_t name_hash() const;

    void sort();
    void on_item_added(std::string_view name) const;

    bool operator==(hashed_netvar_table_name const &name_hash) const;
    bool operator==(std::string_view name) const;
bool operator==(size_t name_hash) const;
};

} // namespace fd