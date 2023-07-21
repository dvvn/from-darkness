#pragma once

#include "object_holder.h"
#include "internal/winapi.h"
#include "render/backend/basic_dx9.h"

namespace fd
{
struct basic_own_dx9_backend : basic_dx9_backend
{
    using basic_dx9_backend::basic_dx9_backend;

    virtual void resize(UINT w, UINT h) = 0;
};

class own_dx9_backend;
FD_OBJECT_FWD(own_dx9_backend, basic_own_dx9_backend);
} // namespace fd