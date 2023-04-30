#include "list.h"

namespace fd
{
// namespace valve
//{
// extern client_ents_list *ents_list;
// }

custom_entity_index::custom_entity_index(game_entity_index index)
    : entity_index(index - 1)
{
}

custom_entity_index::custom_entity_index(size_t index)
    : entity_index(index)
{
}

custom_player_index::custom_player_index(size_t index)
    : custom_entity_index(index - 1)
{
}

custom_player_index::custom_player_index(game_entity_index index)
    : custom_entity_index(index - 1)
{
    assert(index != 0);
    assert(index <= max_players_count);
}

game_entity_index::game_entity_index(custom_entity_index index)
    : entity_index(index + 1)
{
}

player *players_list::get(custom_player_index game_index) const
{
    return access_[game_index];
}

players_list::players_list()
{
#ifdef _DEBUG
    std::fill(std::begin(access_), std::end(access_), nullptr);
#else
    (void)this;
#endif
}

player &players_list::operator[](game_entity_index game_index) const
{
    return *get(game_index);
}

auto players_list::begin() -> players_buffer::iterator
{
    return iterate_.begin();
}

auto players_list::end() -> players_buffer::iterator
{
    return iterate_.end();
}

static bool validate_player_index(size_t index)
{
    return index != 0 && index <= max_players_count;
}

void players_list::on_add_entity(entity_list *ents, handle handle)
{
    auto index = handle.entry_index();
    if (!validate_player_index(index))
        return;
    auto p = &storage_.emplace_back(ents->GetClientEntity(index));
    iterate_.emplace_back(p);
    access_[custom_player_index(index)] = p;
}

static bool operator==(player const *p, player const &p2)
{
    return *p == p2;
}

void players_list::on_remove_entity(entity *ent, custom_player_index index)
{
    assert(validate_player_index(index));

    auto p = std::find(storage_.begin(), storage_.end(), ent);
    if (p == storage_.end())
        return;

    auto it = std::find(iterate_.begin(), iterate_.end(), *p);
    iterate_.erase(it);
    access_[index] = nullptr;
    p->destroy();
}

void players_list::on_remove_entity(entity *ent, game_entity_index index)
{
    if (!validate_player_index(index))
        return;
    on_remove_entity(ent, custom_player_index(index));
}

void players_list::on_remove_entity(entity_list *ents, handle ent_handle)
{
    auto index = ent_handle.entry_index();
    if (!validate_player_index(index))
        return;
    on_remove_entity(ents->GetClientEntity(index), custom_player_index(index));
}

void players_list::update(entity_list *ents)
{
    for (auto p : iterate_)
    {
    }
}
}