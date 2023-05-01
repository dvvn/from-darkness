#pragma once

// #include "iterator.h"
#include "array.h"
#include "range.h"

#include <fd/valve/client_side/entity_list.h>

#include <list>

namespace fd
{
class basic_player_list
{
    using storage_type = std::list<player>;

    storage_type storage_;

  public:
    using iterator = storage_type::iterator;

    /*iterator begin()
    {
        return storage_.begin();
    }

    iterator end()
    {
        return storage_.end();
    }*/

    template <typename T>
    player *add(T &&val)
    {
        return &storage_.emplace_back(std::forward<T>(val));
    }

    template <typename T>
    player *get(T const &val)
    {
        for (auto &p : storage_)
        {
            if (p == val)
                return &p;
        }
        return nullptr;
    }
};

class player_list
{
    using handle      = valve::handle;
    using entity      = valve::client_side::entity;
    using entity_list = valve::client_side::entity_list;

    basic_player_list storage_;
    player_range range_;
    player_array array_;

    using game_index = entity_index<index_source::game>;
    using own_index  = player_index<index_source::own>;

  public:
    void on_add_entity(entity_list *ents, handle handle);

    void on_remove_entity(entity *ent, own_index index);
    void on_remove_entity(entity *ent, game_index index);
    void on_remove_entity(entity_list *ents, handle ent_handle);

    void on_disconnect();

    basic_player_range *range() const;
    basic_player_array *array() const;
};
}