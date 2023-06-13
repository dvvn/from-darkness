#pragma once

#include "basic_array.h"
#include "basic_range.h"
#include "basic_valve_entity_finder.h"

namespace fd
{

class entity_cache
{
    basic_valve_entity_finder *finder_;

  public:
    entity_cache(basic_valve_entity_finder *finder)
        : finder_(finder)
    {
    }

    void add(game_entity_index index);
    void remove(game_entity_index index);
};
}