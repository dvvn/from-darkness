#include <fd/netvars/tables.h>

namespace fd
{
#if 0
netvar_table* netvar_tables::begin()
{
    return storage_.data();
}

netvar_table* netvar_tables::end()
{
    return storage_.data() + storage_.size();
}

netvar_table const* netvar_tables::begin() const
{
    return storage_.data();
}

netvar_table const* netvar_tables::end() const
{
    return storage_.data() + storage_.size();
}
#endif

auto netvar_tables::add(netvar_table&& table) -> pointer
{
    assert(!table.empty());
#ifdef FD_NETVARS_DT_MERGE
    assert(!this->find(table.name()));
#endif
    return &storage_.emplace_back(std::move(table));
}

auto netvar_tables::add(std::string&& name) -> pointer
{
#ifdef FD_NETVARS_DT_MERGE
    assert(!this->find(name));
#endif
    return &storage_.emplace_back(std::move(name));
}

auto netvar_tables::add(std::string_view name) -> pointer
{
#ifdef FD_NETVARS_DT_MERGE
    assert(!this->find(name));
#endif
    return &storage_.emplace_back(name);
}

auto netvar_tables::find(std::string_view name) -> pointer
{
    return std::find(storage_.data(), storage_.data() + storage_.size(), name);
}

auto netvar_tables::find(std::string_view name) const -> const_pointer
{
    return std::find(storage_.data(), storage_.data() + storage_.size(), name);
}

size_t netvar_tables::index_of(const_pointer table) const
{
    assert(std::find(storage_.begin(), storage_.end(), table) != storage_.end());
    return std::distance(storage_.data(), table);
}

void netvar_tables::sort(size_t index)
{
    storage_[index].sort();
}

size_t netvar_tables::size() const
{
    return storage_.size();
}

bool netvar_tables::empty() const
{
    return storage_.empty();
}

auto netvar_tables::make_updater() const -> netvar_tables::updater_fn
{
    return [it = storage_.begin(), end = storage_.end()](basic_netvar_table const*& table) mutable
    {
        auto repeat = it != end;
        if (repeat)
        {
            table = it++.operator->();
            assert(!table->empty());
        }
        return repeat;
    };
}
#if 0
auto netvar_tables::begin() const -> netvar_tables::iterator
{
    return storage_.begin();
}

auto netvar_tables::end() const -> netvar_tables::iterator
{
    return storage_.end();
}

auto netvar_tables::data() const -> netvar_tables::const_pointer
{
    return storage_.data();
}
#endif

//--

#ifdef _DEBUG
netvar_tables_ordered::netvar_tables_ordered()
{
    sortReqests_.reserve(1);
}
#else
netvar_tables_ordered() = default;
#endif

void netvar_tables_ordered::request_sort(const_pointer table)
{
    sortReqests_.push_back(this->index_of(table));
}

void netvar_tables_ordered::sort()
{
    auto sortEnd = std::unique(sortReqests_.begin(), sortReqests_.end());
    for (auto it = sortReqests_.begin(); it != sortEnd; ++it)
        this->sort(*it);
    sortReqests_.clear();
}
} // namespace fd