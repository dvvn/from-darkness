#pragma once

#include "context.h"

namespace fd
{
class render_frame
{
    render_context_ptr ctx_;

  public:
    render_frame(render_context_ptr ctx);
    ~render_frame();

    explicit operator bool() const;

    render_frame &store(auto object)
    {
        (void)object;
        return *this;
    }

    render_frame &store(auto object, auto fn) noexcept
    {
        fn(&object);
        return *this;
    }

    void write();
};
}