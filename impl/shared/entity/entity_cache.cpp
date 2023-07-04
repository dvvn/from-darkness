#include "entity_cache.h"
#include "type_traits.h"
#include "functional/placeholders.h"
#include "iterator/unwrap.h"

#include <algorithm>

namespace fd
{
constexpr auto find_entity_hint = bind(&entity::native, placeholders::_1);

players_array::players_array()
{
    ignore_unused(array_);
}

auto players_array::operator[](own_player_index index) const -> reference
{
    return *array_[index];
}

void players_array::update(player *player, own_player_index index)
{
    array_[index] = player;
}

void players_array::clear()
{
#ifdef _DEBUG
    // for better debug view
    std::fill(array_.begin(), array_.end(), nullptr);
#else
    ignore_unused(array_);
#endif
}

auto players_range::begin() const -> iterator
{
    return range_.data();
}

auto players_range::end() const -> iterator
{
    return range_.data() + range_.size();
}

void players_range::add(player *player)
{
    range_.push_back(player);
}

void players_range::remove(player *player)
{
    range_.erase(std::find_if(range_.begin(), range_.end(), find_entity_hint == player->native()));
}

void players_range::clear()
{
    range_.clear();
}

bool players_range::empty() const
{
    return range_.empty();
}

void entity_cache::reset()
{
    players_array_.clear();
    players_range_.clear();
}

bool entity_cache::add_player(native_entity_index index, bool can_by_null)
{
    if (!validate_player_index(index))
        return false;

    auto ent = find_native_entity_->get(index);
    if (!can_by_null)
        assert(ent != nullptr);
    else if (!ent)
        return false;

    auto player = &players_.emplace_back(ent);
    players_array_.update(player, index);
    players_range_.add(player);
    return true;
}

entity_cache::entity_cache(basic_native_entity_finder *valve_entity_finder, bool sync_wanted)
    : find_native_entity_(valve_entity_finder)
{
    if (sync_wanted)
        request_sync();
    else
        synced_ = true;
}

void entity_cache::add(native_entity_index index)
{
    if (add_player(index, false))
        return;

    (void)0;
    // WIP
}

void entity_cache::remove(native_entity_index index)
{
    if (validate_player_index(index))
    {
        auto player = std::find_if(
            players_.begin(), players_.end(), find_entity_hint == find_native_entity_->get(index));
#ifdef _DEBUG
        assert(player != players_.end());
        players_array_.update(nullptr, index);
#endif
        players_range_.remove(iterator_to_raw_pointer(player));
        if (player->keep())
            player->detach();
        else
            players_.erase(player);
    }
    else
    {
        // WIP
    }
}

bool entity_cache::synced() const
{
    return synced_;
}

void entity_cache::mark_synced()
{
    synced_ = true;
}

void entity_cache::sync(native_entity_index last_entity)
{
    assert(last_entity != 0);
    reset();
    native_entity_index idx = 1;
    for (;;)
    {
        add_player(idx, true);

        if (idx == max_player_count)
            break;
        if (idx == last_entity)
            return;
        ++idx;
    }

#if 0
    for(;;)
    {
        //WIP
        if (idx == last_entity)
            return;
    }
#endif
}

void entity_cache::request_sync()
{
    synced_ = false;
}

void entity_cache::clear()
{
    players_.clear();
    request_sync();
    reset();
}
}