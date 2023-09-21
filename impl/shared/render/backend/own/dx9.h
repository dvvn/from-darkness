#pragma once

#include "internal/winapi.h"
#include "render/backend/basic_dx9.h"

namespace fd
{
template <class T>
struct make_incomplete_object;

struct basic_own_dx9_backend : basic_dx9_backend
{
    using basic_dx9_backend::basic_dx9_backend;

    virtual void resize(UINT w, UINT h) = 0;
};

class own_dx9_backend;

template <>
struct make_incomplete_object<own_dx9_backend> final
{
    basic_own_dx9_backend* operator()() const;
};

} // namespace fd