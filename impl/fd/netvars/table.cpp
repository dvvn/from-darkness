#include <fd/netvars/table.h>

#include <spdlog/spdlog.h>

namespace fd
{
netvar_table::~netvar_table()
{
    std::for_each(storage_.rbegin(), storage_.rend(), [](pointer ptr) { delete ptr; });
}

netvar_table::netvar_table(std::string&& name)
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

netvar_table::netvar_table(netvar_table&& other) noexcept            = default;
netvar_table& netvar_table::operator=(netvar_table&& other) noexcept = default;

std::string_view netvar_table::name() const
{
    return name_;
}

auto netvar_table::find(std::string_view name) const -> const_pointer
{
    assert(!name.empty());
    for (auto* entry : storage_)
    {
        if (entry->name() == name)
            return entry;
    }
    return nullptr;
}

void netvar_table::add(pointer info)
{
    assert(!find(info->name()));
    storage_.emplace_back(info);
}

void netvar_table::sort()
{
    std::stable_sort(storage_.begin(), storage_.end());
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

bool netvar_table::empty() const
{
    return storage_.empty();
}

size_t netvar_table::size() const
{
    return storage_.size();
}

void netvar_table::for_each(for_each_fn const& fn) const
{
    for (auto* i : storage_)
        fn(*i);
}

bool operator==(netvar_table const& table, netvar_table const* externalTable)
{
    return &table == externalTable;
}

bool operator==(netvar_table const& table, std::string_view name)
{
    return table.name() == name;
}

//----

/* bool netvar_table_multi::have_inner() const
{
    return std::holds_alternative<netvar_table_multi>(inner_);
}

netvar_table_multi& netvar_table_multi::inner(std::string&& name)
{
    FD_assert(!have_inner());
    auto& inner = inner_.emplace<netvar_table_multi>();
    inner.construct(std::move(name));
    return inner;
}

netvar_table_multi& netvar_table_multi::inner()
{
    return std::get<netvar_table_multi>(inner_);
}

const netvar_table_multi& netvar_table_multi::inner() const
{
    return std::get<netvar_table_multi>(inner_);
} */
} // namespace fd