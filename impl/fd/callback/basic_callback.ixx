module;

#include <utility>

export module fd.callback.base;
export import fd.functional.invoke;

namespace fd
{
    template <class Buff, typename... Args>
    void _Invoke_callback(Buff* buff, Args&&... args) noexcept
    {
        for (auto& fn : *buff)
            invoke(fn, std::forward<Args>(args)...);
    }

    export template <template <typename...> class Buff, class Fn>
    struct basic_callback : Buff<Fn>
    {
        template <typename... Args>
        void operator()(Args&&... args)
        {
            _Invoke_callback(static_cast<Buff<Fn>*>(this), std::forward<Args>(args)...);
        }

        template <typename... Args>
        void operator()(Args&&... args) const
        {
            _Invoke_callback(static_cast<const Buff<Fn>*>(this), std::forward<Args>(args)...);
        }
    };
} // namespace fd
