#include "hash.h"
#include "table.h"

#include <algorithm>
#include <cassert>

namespace std
{
template <std::same_as<std::string_view> T>
std::string_view get(fd::hashed_netvar_table_name const &p)
{
    return p.second;
}
} // namespace std

namespace fd
{
static bool operator==(hashed_netvar_table_name const &left, hashed_netvar_name const &right)
{
    return left.first == std::get<size_t>(right);
}

hashed_netvar_table_name::hashed_netvar_table_name()
    : hashed_netvar_table_name(std::string_view())
{
}

hashed_netvar_table_name::hashed_netvar_table_name(std::string_view name)
    : std::pair<size_t, std::string>(netvar_hash(name), name)
{
    assert(!name.empty());
}

hashed_netvar_table_name::hashed_netvar_table_name(std::string &&name)
    : std::pair<size_t, std::string>(netvar_hash(name), std::move(name))
{
    assert(!second.empty());
}

hashed_netvar_table_name::hashed_netvar_table_name(const char *name)
    : hashed_netvar_table_name(std::string_view(name))
{
}

hashed_netvar_table_name::hashed_netvar_table_name(hashed_netvar_name const &name)
    : std::pair<size_t, std::string>(name)
{
}

bool hashed_netvar_table_name::operator==(hashed_netvar_table_name const &other) const
{
    return std::get<size_t>(*this) == std::get<size_t>(other);
}

bool hashed_netvar_table_name::operator==(std::string_view other) const
{
    return std::get<std::string_view>(*this) == other;
}



std::string const *hashed_netvar_table_name::operator->() const
{
    return &std::get<std::string>(*this);
}

std::string_view hashed_netvar_table_name::get() const
{
    return std::get<std::string_view>(*this);
}

netvar_table::netvar_table() = default;

netvar_table::netvar_table(hashed_netvar_table_name &&name)
    : name_(std::move(name))
{
}

// netvar_table::netvar_table(char const* name)
//     : name_(name)
//{
// }

netvar_table::netvar_table(netvar_table &&other) noexcept
    : name_(std::move(other.name_))
{
}

netvar_table &netvar_table::operator=(netvar_table &&other) noexcept
{
    name_ = std::move(other.name_);
    return *this;
}

std::string_view netvar_table::name() const
{
    return std::get<std::string_view>(name_);
}

size_t netvar_table::name_hash() const
{
    return std::get<size_t>(name_);
}

void netvar_table::sort()
{
    std::stable_sort(this->begin(), this->end(), [](const_reference left, const_reference right) {
        return left.offset() < right.offset();
    });
}

void netvar_table::on_item_added(std::string_view name) const
{
    (void)this;
    (void)name;
    assert(std::count(this->begin(), this->end(), name) == 1);
}

bool netvar_table::operator==(hashed_netvar_table_name const &name_hash) const
{
    return name_ == name_hash;
}

bool netvar_table::operator==(std::string_view name) const
{
    return this->name() == name;
}

bool netvar_table::operator==(size_t name_hash) const
{
    return std::get<size_t>(name_)==name_hash;
}

#if 0
auto netvar_table::begin() -> iterator
{
    return storage_.begin();
}

auto netvar_table::end() -> iterator
{
    return storage_.end();
}

auto netvar_table::begin() const -> const_iterator
{
    return storage_.begin();
}

auto netvar_table::end() const -> const_iterator
{
    return storage_.end();
}
#endif

} // namespace fd