#pragma once

#include "basic_array.h"
#include "basic_range.h"

namespace fd
{
void on_add_entity(entity_index<index_source::game> index);
void on_remove_entity(entity_index<index_source::game> index);

void reset_player_cache();

basic_player_range *players_range();
basic_player_array *players_array();
}