#pragma once

#include "iterator.h"

#include <fd/valve/client_side/entity_list.h>

#include <boost/container/static_vector.hpp>

#include <array>
#include <list>

namespace fd
{
class entity_index
{
    size_t index_;

  public:
    entity_index(size_t index)
        : index_(index)
    {
    }

    operator size_t() const
    {
        return index_;
    }
};

struct game_entity_index : entity_index
{
    using entity_index::entity_index;
    game_entity_index(struct custom_entity_index index);
};

struct custom_entity_index : entity_index
{
    custom_entity_index(game_entity_index index);

  protected:
    custom_entity_index(size_t index);
};

static constexpr uint8_t max_players_count = 64;

struct custom_player_index : custom_entity_index
{
    explicit custom_player_index(size_t index);
    custom_player_index(game_entity_index index);
};

class players_list
{
    using handle      = valve::handle;
    using entity      = valve::client_side::entity;
    using entity_list = valve::client_side::entity_list;

    using players_buffer = boost::container::static_vector<player *, max_players_count>;

    std::list<player> storage_;
    players_buffer iterate_;
    std::array<player *, max_players_count> access_;

    player *get(custom_player_index game_index) const;

  public:
    players_list();

    player &operator[](game_entity_index game_index) const;
    players_buffer::iterator begin();
    players_buffer::iterator end();

    void on_add_entity(entity_list *ents, handle handle);

    void on_remove_entity(entity *ent, custom_player_index index);
    void on_remove_entity(entity *ent, game_entity_index index);
    void on_remove_entity(entity_list *ents, handle ent_handle);

    void update(entity_list *ents);
};
}