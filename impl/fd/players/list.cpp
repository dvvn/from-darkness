#include "array.h"
#include "list.h"
#include "range.h"

#include <fd/valve/entity_list.h>

#include <algorithm>
#include <list>

namespace fd
{
static class
{
    std::list<player> storage_;

  public:
    /*iterator begin()
    {
        return storage_.begin();
    }

    iterator end()
    {
        return storage_.end();
    }*/

    player *add(void *entity)
    {
        return &storage_.emplace_back(entity);
    }

    player *get(void *entity)
    {
        auto ed = storage_.end();
        auto it = std::find_if(storage_.begin(), ed, [=](player const &p) { return p.native() == entity; });
        return it == ed ? nullptr : &*it;
    }

    bool contains(void *entity) const
    {
        return std::any_of(storage_.begin(), storage_.end(), [=](player const &p) { return p.native() == entity; });
    }

    void clear()
    {
        storage_.clear();
    }
} storage_;

static player_range range_;
static player_array array_;

using valve::get_client_entity;

void on_add_entity(void *entity_list_interface, valve::entity_handle handle)
{
    entity_index<index_source::game> index = get_entity_index(handle);
    if (!validate_player_index(index))
        return;
    auto ent = get_client_entity(entity_list_interface, index);
    assert(!storage_.contains(ent));
    auto p = storage_.add(ent);
    range_.add(p);
    array_.update(index, p);
}

void on_remove_entity(void *entity, player_index<index_source::own> index)
{
    assert(validate_player_index(index, true));
    assert(storage_.contains(entity));
    auto p = storage_.get(entity);
    range_.remove(*p);
#ifdef _DEBUG
    array_.update(index, nullptr);
#endif
    p->destroy();
}

void on_remove_entity(void *entity, entity_index<index_source::game> index)
{
    if (!validate_player_index(index))
        return;
    on_remove_entity(entity, player_index<index_source::own>(index));
}

void on_remove_entity(void *entity_list_interface, valve::entity_handle handle)
{
    entity_index<index_source::game> index = get_entity_index(handle);
    if (!validate_player_index(index))
        return;
    on_remove_entity(get_client_entity(entity_list_interface, index), player_index<index_source::own>(index));
}

void on_disconnect()
{
    // todo: dont full cleanup if map changed
    storage_.clear();
    range_.clear();
}

basic_player_range *players_range()
{
    return &range_;
}

basic_player_array *players_array()
{
    return &array_;
}
}