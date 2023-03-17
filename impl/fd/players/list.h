#pragma once

#include <fd/players/iterator.h>
#include <fd/valve/client_entity_list.h>

#include <boost/container/static_vector.hpp>

#include <array>
#include <list>

namespace fd
{
constexpr size_t max_players_count = 64;

class players_list_global
{
    std::list<player> storage_;

  public:
    player *destroy(valve::client_entity *ent);
    player *add(valve::client_entity *ent);
};

using players_buffer = boost::container::static_vector<player *, max_players_count>;
using players_array  = std::array<player *, max_players_count>;

class players_list
{
    using iterator = players_buffer::iterator;

    players_list_global all_;
    players_buffer buff_;
    players_array arr_;

  public:
    players_list();

    player *operator[](size_t game_index) const;
    iterator begin();
    iterator end();

    void on_add_entity(valve::client_entity_list *ents, valve::base_handle handle);
    void on_remove_entity(valve::client_entity *ent, size_t index = -1);
    void on_remove_entity(valve::client_entity_list *ents, valve::base_handle handle);

    void update(valve::client_entity_list *ents);
};
}