#pragma once

#include "native_entity_finder.h"
#include "players_array.h"
#include "players_range.h"
//
#include "container/array.h"
#include "container/list.h"
#include "container/vector/static.h"

namespace std
{
template <class It, class Fn>
constexpr Fn for_each(It first, It last, Fn func);
}

namespace fd
{
class players_array final : public basic_players_array
{
    array<player *, max_player_count> array_;

  public:
    players_array();

    reference operator[](own_player_index index) const override;

    void update(player *player, own_player_index index);
    void clear();
};

class players_range final : public basic_players_range
{
    static_vector<player *, max_player_count> range_;

  public:
    iterator begin() const override;
    iterator end() const override;

    void add(player *player);
    void remove(player *player);
    void clear();
    bool empty() const;

    template <typename It>
    void assign(It begin, It end)
    {
        if constexpr (std::convertible_to<std::iter_value_t<It>, player *>)
            range_.assign(begin, end);
        else
        {
            range_.clear();
            for_each(begin, end, [this](player &player) { range_.push_back(&player); });
        }
    }
};

class entity_cache
{
    basic_native_entity_finder *find_native_entity_;

    list<player> players_;
    players_array players_array_;
    players_range players_range_;

    bool synced_;

    /**
     * \brief clear everything except root list
     */
    void reset();

    bool add_player(native_entity_index index, bool can_by_null);

  public:
    entity_cache(basic_native_entity_finder *valve_entity_finder, bool sync_wanted);

    void add(native_entity_index index);
    void remove(native_entity_index index);
    /**
     * \brief clear EVERYTHING and wait for sync
     */
    void clear();

    bool synced() const;

    /**
     * \brief clear everything except root list (fo example on map change)
     */
    void request_sync();

    /**
     * \brief sync with game's entity list
     */
    void sync(native_entity_index last_entity);
    void mark_synced();
};
}