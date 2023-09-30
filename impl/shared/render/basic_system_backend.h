#pragma once

#include "basic_backend.h"

namespace fd
{
struct basic_system_backend_info
{
  protected:
    ~basic_system_backend_info() = default;

  public:
    virtual bool minimized() const = 0;
};

struct basic_system_backend : basic_backend
{
    virtual void update(basic_system_backend_info* info) const = 0;
};
}