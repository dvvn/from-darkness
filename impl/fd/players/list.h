#pragma once

// #include "iterator.h"
#include "basic_array.h"
#include "basic_range.h"

#include <fd/valve/entity_handle.h>

namespace fd
{
void on_add_entity(void *entity_list_interface, valve::entity_handle handle);

void on_remove_entity(void *entity, player_index<index_source::own> index);
void on_remove_entity(void *entity, entity_index<index_source::game> index);
void on_remove_entity(void *entity_list_interface, valve::entity_handle handle);

void on_disconnect();

basic_player_range *players_range();
basic_player_array *players_array();
}