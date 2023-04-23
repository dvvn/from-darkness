#pragma once

#include <fd/hooking/hook.h>

namespace fd
{
#undef cdecl
// ReSharper disable CppInconsistentNaming
enum class _x86_call : uint8_t
{
    thiscall_,
    cdecl_,
    fastcall_,
    stdcall_,
    vectorcall_,
};

// ReSharper restore CppInconsistentNaming

template <typename To, typename From = void *>
static To _force_cast(From from, [[maybe_unused]] To toHint = {})
{
    static_assert(sizeof(From) == sizeof(To));

    union
    {
        From from;
        To to;
    } helper;

    helper.from = from;
    return helper.to;
}

template <typename T>
void *decay_fn(T fn)
{
    return _force_cast<void *>(fn);
}

template <class Self, typename Fn>
class hook_original_proxy_member
{
    Self *this_ptr_;
    Fn original_;

  public:
    hook_original_proxy_member(Self *this_ptr, void *original)
        : this_ptr_(this_ptr)
        , original_(_force_cast<Fn>(original))
    {
    }

    template <typename... Args>
    decltype(auto) operator()(Args... args) const
    {
        return (*this_ptr_.*original_)(static_cast<Args>(args)...);
    }
};

template <typename Fn>
class hook_original_proxy
{
    Fn original_;

  public:
    [[deprecated]] //"Prefer native calls"
    hook_original_proxy(void *original)
        : original_(_force_cast<Fn>(original))
    {
    }

    template <typename... Args>
    decltype(auto) operator()(Args... args) const
    {
        return original(static_cast<Args>(args)...);
    }
};

template <typename Sample>
class fn_sample_lazy
{
    union
    {
        Sample sample_;
        void *ptr_;
    };

  public:
    static_assert(sizeof(Sample) == sizeof(void *));

    template <typename T>
    explicit fn_sample_lazy(Sample, T fn)
        : ptr_(reinterpret_cast<void *>(fn))
    {
        static_assert(sizeof(Sample) == sizeof(T));
    }

    template <typename T>
    explicit fn_sample_lazy(T fn)
        : ptr_(reinterpret_cast<void *>(fn))
    {
        static_assert(sizeof(Sample) == sizeof(T));
    }

    Sample get() const
    {
        return sample_;
    }

    operator Sample() const
    {
        return sample_;
    }

    void *ptr() const
    {
        return ptr_;
    }
};

template <typename Sample, typename T>
Sample fn_sample(Sample, T fn)
{
    return fn_sample_lazy<Sample>(fn);
}

template <typename Sample, typename T>
Sample fn_sample(T fn)
{
    return fn_sample_lazy<Sample>(fn);
}

template <class Data, _x86_call Call, typename Ret, class C, typename... Args>
struct hook_callback_proxy_member;

template <class Data, _x86_call Call, typename Ret, class C, typename... Args>
struct hook_callback_proxy_member2;

#define HOOK_CALLBACK_PROXY_MEMBER(_CALL_)                                                                       \
    template <class Data, typename Ret, class C, typename... Args>                                               \
    struct hook_callback_proxy_member<Data, _x86_call::_CALL_##_, Ret, C, Args...> final                         \
    {                                                                                                            \
        using proxy_function_type = Ret (__##_CALL_ hook_callback_proxy_member::*)(Args...);                     \
        using original_proxy      = hook_original_proxy_member<hook_callback_proxy_member, proxy_function_type>; \
        Ret __##_CALL_ proxy(Args... args) noexcept                                                              \
        {                                                                                                        \
            return Data::get_callback()(                                                                         \
                original_proxy(this, Data::original_method), /**/                                                \
                reinterpret_cast<C *>(this),                                                                     \
                std::forward<Args>(args)...);                                                                    \
        }                                                                                                        \
    };

template <class Data, _x86_call Call, typename Ret, typename... Args>
struct hook_callback_proxy;

template <class Data, typename Ret, typename... Args>
struct hook_callback_proxy<Data, _x86_call::thiscall_, Ret, Args...>
{
};

#define HOOK_CALLBACK_PROXY_STATIC(_CALL_)                                     \
    template <class Data, typename Ret, typename... Args>                      \
    struct hook_callback_proxy<Data, _x86_call::_CALL_##_, Ret, Args...> final \
    {                                                                          \
        using function_type = Ret(__##_CALL_ *)(Args...);                      \
        static Ret __##_CALL_ proxy(Args... args) noexcept                     \
        {                                                                      \
            return Data::get_callback()(                                       \
                reinterpret_cast<function_type>(Data::original_method), /**/   \
                std::forward<Args>(args)...);                                  \
        }                                                                      \
    };

#define HOOK_CALLBACK_PROXY_ANY(_CALL_) \
    HOOK_CALLBACK_PROXY_STATIC(_CALL_)  \
    HOOK_CALLBACK_PROXY_MEMBER(_CALL_)

HOOK_CALLBACK_PROXY_ANY(cdecl);
HOOK_CALLBACK_PROXY_ANY(fastcall);
HOOK_CALLBACK_PROXY_ANY(stdcall);
HOOK_CALLBACK_PROXY_ANY(vectorcall);
HOOK_CALLBACK_PROXY_MEMBER(thiscall);

template <typename T>
concept has_destructor = requires(T obj) { obj.~T(); };

template <typename Callback>
class hook_proxy_data
{
    inline static uint8_t callback_[sizeof(Callback)];

  public:
    static Callback &get_callback()
    {
        return reinterpret_cast<Callback &>(callback_);
    }

    inline static void *original_method;
};

template <typename Callback, _x86_call Call, typename Ret, class C, typename... Args>
class hook_callback final
{
  public:
    using proxy_data = hook_proxy_data<Callback>;
    using proxy_type = std::conditional_t<
        std::is_null_pointer_v<C>,
        hook_callback_proxy<proxy_data, Call, Ret, Args...>,
        hook_callback_proxy_member<proxy_data, Call, Ret, C, Args...>>;

  private:
    inline static proxy_type proxy_;
    inline static uint8_t hook_[sizeof(hook)];

    hook &get_hook()
    {
        return reinterpret_cast<hook &>(hook_);
    }

    bool moved_ = false;

  public:
    ~hook_callback()
    {
        (void)proxy_;

        if (moved_)
            return;

        std::destroy_at(&get_hook());
        if constexpr (has_destructor<Callback>)
            std::destroy_at(&proxy_data::get_callback());
    }

    template <typename Fn = void *>
    hook_callback(std::string name, Fn target, Callback callback)
    {
        std::construct_at(&proxy_data::get_callback(), std::move(callback));
        std::construct_at(&get_hook(), std::move(name));
        if (!get_hook().init(decay_fn(target), decay_fn(&proxy_type::proxy)))
            return;
        proxy_data::original_method = get_hook().get_original_method();
    }

    hook_callback(hook_callback const &other)            = delete;
    hook_callback &operator=(hook_callback const &other) = delete;

    hook_callback(hook_callback &&other) noexcept
    {
        other.moved_ = true;
    }

    hook_callback &operator=(hook_callback &&other) noexcept
    {
        other.moved_ = true;
        return *this;
    }

    bool enable()
    {
        return get_hook().enable();
    }

    bool disable()
    {
        return get_hook().disable();
    }
};

template <typename Callback, typename Ret, class C, typename... Args>
hook_callback(std::string, Ret(__thiscall *)(C *, Args...), Callback)
    -> hook_callback<Callback, _x86_call::thiscall_, Ret, C, Args...>;

#define HOOK_CALLBACK_MEMBER(_CALL_)                                      \
    template <typename Callback, typename Ret, class C, typename... Args> \
    hook_callback(std::string, Ret (__##_CALL_ C::*)(Args...), Callback)  \
        -> hook_callback<Callback, _x86_call::_CALL_##_, Ret, C, Args...>;

#define HOOK_CALLBACK_STATIC(_CALL_)                                 \
    template <typename Callback, typename Ret, typename... Args>     \
    hook_callback(std::string, Ret(__##_CALL_ *)(Args...), Callback) \
        -> hook_callback<Callback, _x86_call::_CALL_##_, Ret, nullptr_t, Args...>;

#define HOOK_CALLBACK_ANY(_CALL_) \
    HOOK_CALLBACK_STATIC(_CALL_)  \
    HOOK_CALLBACK_MEMBER(_CALL_)

HOOK_CALLBACK_ANY(cdecl);
HOOK_CALLBACK_ANY(fastcall);
HOOK_CALLBACK_ANY(stdcall);
HOOK_CALLBACK_ANY(vectorcall);
HOOK_CALLBACK_MEMBER(thiscall);

} // namespace fd