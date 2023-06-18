#include "core.h"
#include "entity_cache.h"

#include <algorithm>

namespace fd
{
constexpr auto find_cached_entity_hint = bind(&cached_entity::native, placeholders::_1);

cached_players_array::cached_players_array()
{
    ignore_unused(array_);
}

auto cached_players_array::operator[](own_player_index index) const -> pointer
{
    return array_[index];
}

void cached_players_array::update(cached_player *player, own_player_index index)
{
    array_[index] = player;
}

void cached_players_array::clear()
{
#ifdef _DEBUG
    // for better debug view
    std::fill(array_.begin(), array_.end(), nullptr);
#else
    ignore_unused(array_);
#endif
}

auto cached_players_range::begin() const -> iterator
{
    return range_.data();
}

auto cached_players_range::end() const -> iterator
{
    return range_.data() + range_.size();
}

void cached_players_range::add(cached_player *player)
{
    range_.push_back(player);
}

void cached_players_range::remove(cached_player *player)
{
    range_.erase(std::find_if(range_.begin(), range_.end(), find_cached_entity_hint == player->native()));
}

void cached_players_range::clear()
{
    range_.clear();
}

entity_cache::entity_cache(basic_native_entity_finder *valve_entity_finder, bool already_synced)
    : find_native_entity_(valve_entity_finder)
{
    if (already_synced)
        synced_ = true;
    else
        sync_request();
}

void entity_cache::add(game_entity_index index)
{
    if (validate_player_index(index))
    {
        auto player = &players_.emplace_back(find_native_entity_->get(index));
        players_array_.update(player, index);
        players_range_.add(player);
    }
    else
    {
        // WIP
    }
}

void entity_cache::remove(game_entity_index index)
{
    if (validate_player_index(index))
    {
        auto player = std::find_if(
            players_.begin(), players_.end(), find_cached_entity_hint == find_native_entity_->get(index));
#ifdef _DEBUG
        assert(player != players_.end());
        players_array_.update(nullptr, index);
#endif
        players_range_.remove(iterator_to_raw_pointer(player));
        player->detach();
    }
    else
    {
        // WIP
    }
}

void entity_cache::sync(game_entity_index last_entity)
{
    if (synced_)
        return;

    for (game_entity_index::size_type idx = 0;;)
    {
        add(idx);
        if (idx == last_entity)
            break;
        ++idx;
    }
    synced_ = true;
}

void entity_cache::sync_request()
{
    synced_ = false;

    players_array_.clear();
    players_range_.clear();
}

void entity_cache::clear()
{
    players_.clear();
    sync_request();
}
}