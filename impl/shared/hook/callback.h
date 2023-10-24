#pragma once
#include "noncopyable.h"
#include "functional/ignore.h"

#include <atomic>

namespace fd
{
class basic_hook_callback : public noncopyable
{
    static constexpr auto order = std::memory_order_relaxed;
    using value_type            = std::atomic<size_t>;

    value_type called_;

  protected:
    ~basic_hook_callback();

  public:
    basic_hook_callback();

    void enter();
    void exit();
};

template <class T, bool>
class hook_callback_thread_protector;

//template <class T, bool V>
//class hook_callback_thread_protector<T const, V>;

template <class T>
class hook_callback_thread_protector<T, true> final : public noncopyable
{
    T* callback_;

  public:
    ~hook_callback_thread_protector()
    {
        callback_->exit();
    }

    hook_callback_thread_protector(T* callback)
        : callback_(callback)
    {
        callback->enter();
    }

    hook_callback_thread_protector(T& callback)
        : hook_callback_thread_protector(std::addressof(callback))
    {
    }
};

template <class T>
class hook_callback_thread_protector<T, false> final : public noncopyable
{
  public:
    hook_callback_thread_protector(auto&)
    {
    }
};

namespace detail
{
template <class T>
concept hook_callback_have_enter_exit = requires(T obj) {
    obj.enter();
    obj.exit();
};
}

template <class T>
inline constexpr bool hook_callback_need_protect = detail::hook_callback_have_enter_exit<T>;

template <class T>
hook_callback_thread_protector(T*) -> hook_callback_thread_protector<T, hook_callback_need_protect<T>>;
template <class T>
hook_callback_thread_protector(T&) -> hook_callback_thread_protector<T, hook_callback_need_protect<T>>;
} // namespace fd