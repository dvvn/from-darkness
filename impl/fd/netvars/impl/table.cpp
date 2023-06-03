#include "table.h"

#include <algorithm>
#include <cassert>

namespace fd
{
netvar_table::netvar_table(netvar_table &&other) noexcept
    : name_(move(other.name_))
{
}

netvar_table &netvar_table::operator=(netvar_table &&other) noexcept
{
    name_ = move(other.name_);
    return *this;
}

std::string_view netvar_table::name() const
{
    return get<std::string_view>(name_);
}

size_t netvar_table::name_hash() const
{
    return get<size_t>(name_);
}

void netvar_table::sort()
{
    stable_sort(this->begin(), this->end(), [](const_reference left, const_reference right) {
        return left.offset() < right.offset();
    });
}

void netvar_table::on_item_added(std::string_view name) const
{
    (void)this;
    (void)name;
    assert(count(this->begin(), this->end(), name) == 1);
}

bool netvar_table::operator==(_const<hashed_name_view &> name_hash) const
{
    return name_ == name_hash;
}

bool netvar_table::operator==(std::string_view name) const
{
    return this->name() == name;
}

bool netvar_table::operator==(size_t name_hash) const
{
    return get<size_t>(name_) == name_hash;
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