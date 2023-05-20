#include "tables.h"

#include <algorithm>
#include <cassert>
#include <functional>

namespace fd
{
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

void netvar_tables_ordered::request_sort(const_iterator table)
{
    // write_sort_value(std::in_place_type<sort_value>, this, table, sort_reqests_);
    if constexpr (std::ranges::random_access_range<netvar_tables>)
        sort_reqests_.emplace_back(std::distance(cbegin(), table)); // store offset
    else
        sort_reqests_.emplace_back(table->name_hash());
}

static void do_sort(netvar_table &table)
{
    using ref_t = netvar_table::const_reference;
    std::stable_sort(table.begin(), table.end(), [](ref_t l, ref_t r) {
        //
        return l.offset() < r.offset();
    });
}

template <class Rng>
concept can_find = requires(Rng &rng) { rng.find(size_t()); };

template <class Rng>
static void do_sort(Rng &rng, size_t value)
{
    if constexpr (std::ranges::random_access_range<Rng>) // offset
        do_sort(rng[value]);
    else if constexpr (can_find<Rng>) // name_hash
        do_sort(rng.find(value));
    else
        do_sort(*std::find(rng.begin(), rng.end(), value));
}

void netvar_tables_ordered::sort()
{
    auto bg = sort_reqests_.begin();
    auto ed = std::unique(bg, sort_reqests_.end());
    std::for_each(bg, ed, [&](size_t val) { do_sort(*this, val); });
    sort_reqests_.resize(0);
}
} // namespace fd