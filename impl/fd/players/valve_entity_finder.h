#pragma once

#include "basic_valve_entity_finder.h"

#include <fd/valve/entity_list.h>

namespace fd
{
class valve_entity_finder final : public basic_valve_entity_finder
{
    valve::entity_list *list_;

  public:
    valve_entity_finder(valve::entity_list *list)
        : list_(list)
    {
    }

    void *get(game_entity_index index) override
    {
        return list_->get_client_entity(index);
    }
};
} // namespace fd