#pragma once

#include <fd/players/iterator.h>
#include <fd/valve/client_side/entity_list.h>

#include <boost/container/static_vector.hpp>

#include <array>
#include <list>

namespace fd
{
constexpr size_t max_players_count = 64;

class players_list_global
{
    using entity = valve::client_side::entity;

    std::list<player> storage_;

  public:
    player *destroy(entity *ent);
    player *add(entity *ent);
};

using players_buffer = boost::container::static_vector<player *, max_players_count>;
using players_array  = std::array<player *, max_players_count>;

class players_list
{
    using handle      = valve::handle;
    using entity      = valve::client_side::entity;
    using entity_list = valve::client_side::entity_list;

    using iterator = players_buffer::iterator;

    players_list_global all_;
    players_buffer buff_;
    players_array arr_;

  public:
    players_list();

    player &operator[](size_t game_index) const;
    iterator begin();
    iterator end();

    void on_add_entity(entity_list *ents, handle handle);
    void on_remove_entity(entity *ent, size_t index = -1);
    void on_remove_entity(entity_list *ents, handle ent_handle);

    void update(entity_list *ents);
};
}