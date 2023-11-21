#pragma once

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

template <typename Fn>
void* get_vfunc(Fn abstract_function, void* instance)
#if 0
    requires(std::is_member_function_pointer_v<Fn>)
#else
    requires requires { typename function_info<Fn>::object_type; }
#endif
{
    using call_type = typename function_info<Fn>::call_type;
    auto function   = unsafe_cast<void*>(abstract_function);
    return get_vfunc<call_type>(function, instance);
}

void* get_vfunc(size_t index, void* instance);

template <typename Fn, size_t FnSize = sizeof(Fn)>
struct vfunc
{
    // WIP
};

template <typename Fn>
struct vfunc<Fn, sizeof(void*)>
{
    using function_type = Fn;

    using info = function_info<function_type>;

    using object_type = typename info::object_type;

  private:
    union
    {
        function_type func_;
        void* func_ptr_;
    };

    basic_vtable<object_type> source_;

  public:
    vfunc(size_t const index, object_type* instance)
        : func_ptr_(get_vfunc(index, instance))
        , source_(instance)
    {
    }

    vfunc(function_type function, object_type* instance)
        : func_ptr_(get_vfunc(function, instance))
        , source_(instance)
    {
    }

    vfunc(function_type function, object_type* instance, std::in_place_t)
        : func_(function)
        , source_(instance)
    {
    }

    template <std::same_as<function_type> T>
    operator T() const
    {
        return func_;
    }

    template <typename... Args>
    auto operator()(Args&&... args) const noexcept(info::no_throw) -> typename
#ifdef _DEBUG
        // ReSharper disable once CppUseTypeTraitAlias
        std::invoke_result<function_type, object_type*, Args&&...>::type
#else
        info::return_type
#endif
    {
        using std::invoke;
        return invoke(func_, source_.instance(), std::forward<Args>(args)...);
    }
};

template <typename Func, typename... Next>
vfunc(Func fn, Next...) -> vfunc<Func>;
} // namespace fd