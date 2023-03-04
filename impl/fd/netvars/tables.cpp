#include <fd/netvars/tables.h>

#include <algorithm>
#include <cassert>

namespace fd
{
static auto _find_raw(auto* rng, std::string_view val) -> decltype(rng->data())
{
    auto e = rng->end();
    for (auto it = rng->begin(); it != e; ++it)
    {
        if (*it == val)
            return it.operator->();
    }
    return nullptr;
}

auto netvar_tables::find(std::string_view name) -> pointer
{
    return _find_raw(this, name);
}

auto netvar_tables::find(std::string_view name) const -> const_pointer
{
    return _find_raw(this, name);
}

static bool _constains(auto* rng, void const* ptr)
{
    for (auto& item : *rng)
    {
        if (&item == ptr)
            return true;
    }
    return false;
}

size_t netvar_tables::index_of(const_pointer table) const
{
    assert(_constains(this, table));
    return std::distance(this->data(), table);
}

void netvar_tables::sort(size_t index)
{
    this->operator[](index).sort();
}

void netvar_tables::on_item_added(const_reference table) const
{
    (void)table;
    (void)this;
    assert(std::count(this->begin(), this->end(), table.name()) == 1);
}

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
    sortReqests_.resize(0);
}
} // namespace fd