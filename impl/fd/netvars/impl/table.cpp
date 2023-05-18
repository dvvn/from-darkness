#include "table.h"

#include <algorithm>
#include <cassert>

namespace fd
{
netvar_table::netvar_table(std::string &&name)
    : name_(std::move(name))
{
    assert(!name_.empty());
}

netvar_table::netvar_table(std::string_view name)
    : name_(name)
{
    assert(!name.empty());
}

// netvar_table::netvar_table(char const* name)
//     : name_(name)
//{
// }

netvar_table::netvar_table(netvar_table &&other) noexcept            = default;
netvar_table &netvar_table::operator=(netvar_table &&other) noexcept = default;

void netvar_table::set_name(std::string &&name)
{
    assert(name_.empty());
    name_ = std::move(name);
}

std::string_view netvar_table::name() const
{
    return name_;
}



auto netvar_table::find(std::string_view name) const -> const_pointer
{
    assert(!name.empty());
    auto ed = this->end();
#if 1
    auto it = std::find_if(this->begin(), ed, [=](netvar_info const &info) {
        //
        return info.name() == name;
    });
#else
    auto it = std::find(this->begin(), ed, name);
#endif
    return it == ed ? nullptr : &*it;
}

void netvar_table::sort()
{
    std::stable_sort(this->begin(), this->end());
}

void netvar_table::on_item_added(std::string_view name) const
{
    (void)this;
    (void)name;
    assert(std::count(this->begin(), this->end(), name) == 1);
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