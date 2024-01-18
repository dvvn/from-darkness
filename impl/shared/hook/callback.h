#pragma once
#include "functional/invoke_on.h"

#include <boost/noncopyable.hpp>

#include <atomic>

namespace fd
{
class basic_hook_callback : public boost::noncopyable
{
    std::atomic_size_t called_;

  protected:
    ~basic_hook_callback();

  public:
    basic_hook_callback();

    void enter() noexcept;
    void exit() noexcept;
};

template <typename Callback>
extern Callback global_hook_callback;
template <typename Callback>
inline Callback* global_hook_callback<Callback*>;
template <typename Callback>
inline void* global_hook_original_func;

namespace detail
{
template <typename Callback, typename... Args>
decltype(auto) invoke_hook_callback(Args&&... args)
{
    if constexpr (!std::derived_from<Callback, basic_hook_callback>)
    {
        return global_hook_callback<Callback>(std::forward<Args>(args)...);
    }
    else
    {
#ifdef _DEBUG
        using fn_ret = std::invoke_result_t<Callback&, Args&&...>;
        if constexpr (std::is_void_v<fn_ret>)
        {
            global_hook_callback<Callback>.enter();
            global_hook_callback<Callback>(std::forward<Args>(args)...);
            global_hook_callback<Callback>.exit();
        }
        else
#endif
        {
            auto const lazy_exit = make_invoke_on<invoke_on_state::construct | invoke_on_state::destruct>(
                []<invoke_on_state State>(std::integral_constant<invoke_on_state, State>) {
                    if constexpr (State == invoke_on_state::construct)
                        global_hook_callback<Callback>.enter();
                    else if constexpr (State == invoke_on_state::destruct)
                        global_hook_callback<Callback>.exit();
                });
            return global_hook_callback<Callback>(std::forward<Args>(args)...);
        }
    }
}
} // namespace detail
} // namespace fd