﻿#pragma once
#include "entity_index.h"

namespace fd
{
struct cached_entity
{
    using native_pointer = void *;

  private:
    native_pointer entity_;

  public:
    cached_entity(void *entity);

    native_pointer native() const;

    bool valid() const;
    /**
     * \brief detach from game's data, but keep for future use
     */
    void detach();
    /**
     * \brief should we remove this entity from cache?
     */
    bool keep() const;

    game_entity_index index() const;
};
} // namespace fd