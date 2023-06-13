#include "array.h"
#include "cache.h"
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



void entity_cache::add(game_entity_index index)
{
#if 0
    if (!validate_player_index(index))
        return;
    auto ent = finder_->get(index);
    assert(!storage_.contains(ent));
    auto p = storage_.add(ent);
    range_.add(p);
    array_.update(index, p);
#endif
}

void entity_cache::remove(game_entity_index index)
{
#if 0
    if constexpr (Validate)
        assert(validate_player_index(index, true));
    assert(storage_.contains(entity));
    auto p = storage_.get(entity);
    range_.remove(*p);
#ifdef _DEBUG
    array_.update(index, nullptr);
#endif
    p->destroy();
#endif
}
}