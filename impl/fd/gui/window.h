#pragma once

#include <fd/render/context_fwd.h>

namespace fd
{
class window
{
    render_context_ptr ctx_;

  public:
    window(render_context_ptr ctx)
        : ctx_(ctx)
    {
    }

    ~window()
    {
    }

    /*template <class T, typename Callback>
    void store(Callback fn)
    {
        T object;
        fn(&object);
    }

    template <class T, typename Callback>
    void store(T object, Callback fn)
    {
        fn(&object);
    }*/

    template <typename T>
    void store(T object, auto fn)
    {
        (void)this;
        fn(&object);
    }

    /*template <typename Callback>
    void checkbox(ImStrv text, bool &value, Callback callback)
    {
    }*/
};
} // namespace fd