#include <fd/players/list.h>

#include <functional>

namespace fd
{
// namespace valve
//{
// extern client_entity_list *entity_list;
// }

player *players_list_global::destroy(valve::client_entity *ent)
{
    assert(ent != nullptr);
    for (auto &p : storage_)
    {
        if (p == ent)
        {
            p.ptr_ = nullptr;
            return &p;
        }
    }
    return nullptr;
}

player *players_list_global::add(valve::client_entity *ent)
{
    return &storage_.emplace_back(ent);
}

players_list::players_list()
{
    arr_.fill(nullptr);
}

player *players_list::operator[](size_t game_index) const
{
    return arr_[game_index - 1];
}

auto players_list::begin() -> iterator
{
    return buff_.begin();
}

auto players_list::end() -> iterator
{
    return buff_.end();
}

static bool validate_player_index(size_t index)
{
    return index != 0 && index <= 64;
}

void players_list::on_add_entity(valve::client_entity_list *ents, valve::base_handle handle)
{
    auto index = handle.GetEntryIndex();
    if (!validate_player_index(index))
        return;
    auto p = all_.add(ents->GetClientEntity(index));
    buff_.emplace_back(p);
    arr_[index] = p;
}

void players_list::on_remove_entity(valve::client_entity *ent, size_t index)
{
    if (index == -1)
    {
        index = ent->EntIndex();
        if (!validate_player_index(index))
            return;
    }
    auto p = all_.destroy(ent);
    if (!p)
        return;
    buff_.erase(std::find_if(buff_.begin(), buff_.end(), [=](player *p1) { return *p == *p1; }));
    arr_[index] = nullptr;
}

void players_list::on_remove_entity(valve::client_entity_list *ents, valve::base_handle handle)
{
    auto index = handle.GetEntryIndex();
    if (!validate_player_index(index))
        return;
    on_remove_entity(ents->GetClientEntity(index), index);
}

void players_list::update(valve::client_entity_list *ents)
{
    for (auto p : buff_)
    {
    }
}
}