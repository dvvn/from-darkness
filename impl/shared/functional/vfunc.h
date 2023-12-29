#pragma once
#include "functional/basic_vfunc.h"
#include "functional/basic_vtable.h"
#include "functional/call_traits.h"

namespace fd
{
template <class Call_T>
void* get_vfunc(void* abstract_function, void* instance);

#define GET_VFUNC(_CCV_, ...) \
    template <>               \
    void* get_vfunc<_CCV_T(_CCV_)>(void* abstract_function, void* instance);

#ifdef _MSC_VER
_MEMBER_CALL(GET_VFUNC, , , )
#else

#endif
#undef GET_VFUNC

template <typename Func>
void* get_vfunc(Func abstract_function, void* instance)
#if 0
    requires(std::is_member_function_pointer_v<Func>)
#else
    requires requires { typename function_info<Func>::object_type; }
#endif
{
    using call_type = typename function_info<Func>::call_type;
    auto function   = unsafe_cast<void*>(abstract_function);
    return get_vfunc<call_type>(function, instance);
}

inline void** get_vtable(void* instance)
{
    return *static_cast<void***>(instance);
}

inline void* get_vfunc(size_t const index, void* instance)
{
    return get_vtable(instance)[index];
}

namespace detail
{
template <class T>
class pointer_extractor
{
    T* value_;

  public:
    pointer_extractor(T* ptr)
        : value_{ptr}
    {
    }

    template <class Packed>
    pointer_extractor(Packed&& ptr_packed) requires requires { static_cast<T*>(ptr_packed.get()); }
        : value_{ptr_packed.get()}
    {
    }

    operator T*() const
    {
        return value_;
    }
};

template <typename Func, size_t FnSize = sizeof(Func)>
class vfunc_impl;

template <typename Func>
class vfunc_impl<Func, sizeof(void*)>
{
    using fn_info            = function_info<Func>;
    using object_type        = typename fn_info::object_type;
    using instance_extractor = pointer_extractor<object_type>;

    union
    {
        Func func_;
        void* func_ptr_;
    };

    basic_vtable<object_type> source_;

  public:
    vfunc_impl(size_t const index, instance_extractor instance)
        : func_ptr_(get_vfunc(index, instance))
        , source_(instance)
    {
    }

    vfunc_impl(Func function, instance_extractor instance)
        : func_ptr_(get_vfunc(function, instance))
        , source_(instance)
    {
    }

    vfunc_impl(Func function, instance_extractor instance, std::in_place_t)
        : func_(function)
        , source_(instance)
    {
    }

    template <std::same_as<Func> T>
    operator T() const
    {
        return func_;
    }

    template <typename... Args>
    auto operator()(Args&&... args) const noexcept(fn_info::no_throw) -> typename
#ifdef _DEBUG
        // ReSharper disable once CppUseTypeTraitAlias
        std::invoke_result<Func, object_type*, Args&&...>::type
#else
        info::return_type
#endif
    {
        using std::invoke;
        return invoke(func_, source_.instance(), std::forward<Args>(args)...);
    }
};
} // namespace detail

template <typename Func>
struct vfunc final : detail::vfunc_impl<Func>, basic_vfunc<Func>
{
    using detail::vfunc_impl<Func>::vfunc_impl;
};

template <typename Func, typename... Next>
vfunc(Func fn, Next...) -> vfunc<Func>;
} // namespace fd