#pragma once

#include "entity_index.h"

namespace fd
{
struct basic_native_entity_finder
{
    virtual void *get(game_entity_index index) = 0;
};

template <typename Fn>
class native_entity_finder : public basic_native_entity_finder
{
    Fn fn_;

  public:
    native_entity_finder(Fn fn)
        : fn_(std::move(fn))
    {
    }

    void *get(game_entity_index index) override
    {
        return fn_(index);
    }
};
}