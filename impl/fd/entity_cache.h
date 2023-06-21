#pragma once

#include "cached_players_array.h"
#include "cached_players_range.h"
#include "entity_cache/native_entity_finder.h"
#include "tool/array.h"
#include "tool/functional.h"
#include "tool/list.h"
#include "tool/vector.h"

namespace std
{
template <class _InIt, class _Fn>
constexpr _Fn for_each(_InIt _First, _InIt _Last, _Fn _Func);
}

namespace fd
{
class cached_players_array : public basic_cached_players_array
{
    array<cached_player *, max_player_count> array_;

  public:
    cached_players_array();

    pointer operator[](own_player_index index) const override;

    void update(cached_player *player, own_player_index index);
    void clear();
};

class cached_players_range : public basic_cached_players_range
{
    static_vector<cached_player *, max_player_count> range_;

  public:
    iterator begin() const override;
    iterator end() const override;

    void add(cached_player *player);
    void remove(cached_player *player);
    void clear();
    bool empty() const;

    template <typename It>
    void assign(It begin, It end)
    {
        if constexpr (std::convertible_to<std::iter_value_t<It>, cached_player *>)
            range_.assign(begin, end);
        else
        {
            range_.clear();
            for_each(begin, end, [this](cached_player &player) { range_.push_back(&player); });
        }
    }
};

class entity_cache
{
    basic_native_entity_finder *find_native_entity_;

    list<cached_player> players_;
    cached_players_array players_array_;
    cached_players_range players_range_;

    bool synced_;

    /**
     * \brief clear everything except root list
     */
    void reset();

    bool add_player(game_entity_index index, bool can_by_null);

  public:
    entity_cache(basic_native_entity_finder *valve_entity_finder, bool sync_wanted);

    void add(game_entity_index index);
    void remove(game_entity_index index);
    void clear();

    bool synced() const;

    /**
     * \brief clear everything except root list (fo example on map change)
     */
    void request_sync();
    /**
     * \brief clear EVERYTHING and wait for sync
     */
    /**
     * \brief sync with game's entity list
     */
    void sync(game_entity_index last_entity);
    void mark_synced();
};
}