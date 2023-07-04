#pragma once

#include "entity_index.h"

namespace fd
{
class basic_native_entity_finder
{
  protected:
    ~basic_native_entity_finder() = default;

  public:
    virtual void *get(native_entity_index index) = 0;
};

template <typename Fn>
class native_entity_finder final: public basic_native_entity_finder
{
    Fn fn_;

  public:
    native_entity_finder() = default;

    native_entity_finder(Fn fn)
        : fn_(std::move(fn))
    {
    }

    void set(Fn fn)
    {
        fn_ = std::move(fn);
    }

    void *get(native_entity_index index) override
    {
        return fn_(index);
    }
};
}