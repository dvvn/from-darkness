#include "list.h"

namespace fd
{
// namespace valve
//{
// extern client_ents_list *ents_list;
// }

void player_list::on_add_entity(entity_list *ents, handle handle)
{
    game_index index = handle.entry_index();
    if (!validate_player_index(index))
        return;

    auto ent = ents->GetClientEntity(index);
    assert(!storage_.get(ent));
    auto p = storage_.add(ent);
    range_.add(p);
    array_.update(index, p);
}

void player_list::on_remove_entity(entity *ent, own_index index)
{
    assert(validate_player_index(index, true));

    auto p = storage_.get(*ent);
    assert(p != nullptr);
    range_.remove(*p);
    array_.update(index, nullptr);
    p->destroy();
}

void player_list::on_remove_entity(entity *ent, game_index index)
{
    if (!validate_player_index(index))
        return;
    on_remove_entity(ent, own_index(index));
}

void player_list::on_remove_entity(entity_list *ents, handle ent_handle)
{
    game_index index = ent_handle.entry_index();
    if (!validate_player_index(index))
        return;
    on_remove_entity(ents->GetClientEntity(index), own_index(index));
}

void player_list::on_disconnect()
{
    // todo: dont full cleanup if map changed

    using std::swap;
    player_list empty;
    swap(*this, empty);
}

basic_player_range *player_list::range() const
{
    return const_cast<player_range *>(&range_);
}

basic_player_array *player_list::array() const
{
    return const_cast<player_array *>(&array_);
}
}