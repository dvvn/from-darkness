#pragma once

#include "object_holder.h"
#include "render/backend/basic_win32.h"

namespace fd
{
template <class T>
struct make_incomplete_object;

struct basic_own_win32_backend : basic_win32_backend
{
    using basic_win32_backend::basic_win32_backend;

    virtual void peek()         = 0;
    virtual void close()        = 0;
    virtual bool closed() const = 0;
};

class own_win32_backend;

template <>
struct make_incomplete_object<own_win32_backend> final
{
    basic_own_win32_backend* operator()() const;
};
}