#include "tables.h"

#include <algorithm>
#include <cassert>
#include <functional>

namespace fd
{
static bool operator==(netvar_table const &table, std::string_view name)
{
    return table.name() == name;
}

static bool operator==(netvar_table const &table, void const *ptr)
{
    return &table == ptr;
}

template <typename T>
static auto _find_raw(T *rng, std::string_view val) -> decltype(&*rng->begin())
{
    auto ed = rng->end();
    auto it = std::find(rng->begin(), ed, val);
    if (it != ed)
        return &*it;
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

template <typename T>
static bool _constains(T *rng, void const *ptr)
{
    using op_t = bool (*)(netvar_table const &, void const *);
    op_t op    = operator==;
    return std::any_of(rng->begin(), rng->end(), std::bind_back(op, ptr));
}

template <typename T>
static size_t _index_of(T *storage, void const *target)
{
    auto bg = storage->begin();
    auto it = std::find(bg, storage->end(), target);
    return std::distance(bg, it);
}

size_t netvar_tables::index_of(const_pointer table) const
{
    assert(_constains(this, table));
    return _index_of(this, table);
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
    sort_reqests_.reserve(1);
}
#else
netvar_tables_ordered() = default;
#endif

template <typename T>
static void write_sort_value(
    std::in_place_type_t<size_t>,
    T *storage,
    typename T::const_pointer target,
    auto &sort_reqests)
{
    sort_reqests.emplace_back(_index_of(storage, target));
}

template <typename T>
static void write_sort_value(
    std::in_place_type_t<typename T::const_pointer>,
    T *,
    typename T::const_pointer target,
    auto &sort_reqests)
{
    sort_reqests.emplace_back(target);
}

void netvar_tables_ordered::request_sort(const_pointer table)
{
    write_sort_value(std::in_place_type<sort_value>, this, table, sort_reqests_);
}

static void _sort(auto *storage, size_t hint)
{
    storage[hint].sort();
}

template <typename T>
static void _sort(T *storage, typename T::const_pointer hint)
{
    hint->sort();
}

void netvar_tables_ordered::sort()
{
    auto bg = sort_reqests_.begin();
    auto ed = std::unique(bg, sort_reqests_.end());
    std::for_each(bg, ed, [&](sort_value val) { _sort(this, val); });
    sort_reqests_.resize(0);
}
} // namespace fd