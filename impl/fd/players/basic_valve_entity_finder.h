#pragma once

#include "index.h"

namespace fd
{
struct basic_valve_entity_finder
{
    virtual void *get(game_entity_index index) = 0;
};

}