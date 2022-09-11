module;

#include <concepts>

export module fd.functional.lazy_invoke;
import fd.functional.invoke;

template <typename T>
concept can_be_null = requires(T obj) {
                          !obj;
                          obj = nullptr;
                      };

using fd::invoke;

template <typename Fn, bool = can_be_null<Fn>>
struct lazy_invoke;

template <typename Fn>
class lazy_invoke_base
{
    Fn fn_;

    friend struct lazy_invoke<Fn>;

  public:
    template <typename Fn1>
    constexpr lazy_invoke_base(Fn1&& fn)
        : fn_(std::forward<Fn1>(fn))
    {
    }

    lazy_invoke_base(const lazy_invoke_base&)            = delete;
    lazy_invoke_base& operator=(const lazy_invoke_base&) = delete;
};

template <typename Fn>
struct lazy_invoke<Fn, true> : lazy_invoke_base<Fn>
{
    constexpr ~lazy_invoke()
    {
        if (this->fn_)
            invoke(this->fn_);
    }

    using lazy_invoke_base<Fn>::lazy_invoke_base;
    using lazy_invoke_base<Fn>::operator=;

    constexpr void reset()
    {
        this->fn_ = nullptr;
    }
};

template <typename Fn>
struct lazy_invoke<Fn, false> : lazy_invoke_base<Fn>
{
  private:
    bool valid_ = true;

  public:
    constexpr ~lazy_invoke()
    {
        if (valid_)
            invoke(this->fn_);
    }

    using lazy_invoke_base<Fn>::lazy_invoke_base;
    using lazy_invoke_base<Fn>::operator=;

    constexpr void reset()
    {
        valid_ = false;
    }
};

template <typename Fn>
lazy_invoke(Fn&&) -> lazy_invoke<std::decay_t<Fn>>;

export namespace fd
{
    using ::lazy_invoke;
}
