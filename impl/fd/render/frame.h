#pragma once

#include "construct_args.h"
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

    template <typename T, typename... Args>
    render_frame &store(construct_args<Args...> args, auto fn) noexcept
    {
        manual_construct_t<T> object;
        apply_construct_args_front(&object, args, ctx_);
        fn(&extract_manual_object(object));

        return *this;
    }

    template <typename T>
    render_frame &store(T object, auto fn) noexcept
    {
        fn(&object);

        return *this;
    }

    void write();
};
}