#pragma once

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

template <class Object, size_t Buff>
class basic_vfunc
{
    uint8_t function_[Buff];
    Object* instance_;

  public:
    template <typename Fn>
    basic_vfunc(Fn function, Object* instance)
        : instance_(instance)
    {
        static_assert(sizeof(Fn) == Buff);
        memcpy(function_, function, Buff);
    }

    Object* instance() const
    {
        return instance_;
    }

  protected:
    void*& get() const&
    {
        return reinterpret_cast<void*&>(remove_const(function_));
    }

    operator void* const() const&
    {
        return get();
    }
};

template <class Object>
class basic_vfunc<Object, sizeof(void*)>
{
    void* function_;
    Object* instance_;

  public:
    template <typename Fn>
    basic_vfunc(Fn function, Object* instance)
        : function_(unsafe_cast<void*>(function))
        , instance_(instance)
    {
    }

    void* get() const
    {
        return function_;
    }

    operator void*() const
    {
        return function_;
    }

    Object* instance() const
    {
        return instance_;
    }
};

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
    using return_type = typename info::return_type;

  private:
    union
    {
        function_type func_;
        void* func_ptr_;
    };

    object_type* instance_;

  public:
    vfunc(size_t const index, object_type* instance)
        : func_ptr_(get_vfunc(index, instance))
        , instance_(instance)
    {
    }

    vfunc(function_type function, object_type* instance)
        : func_ptr_(get_vfunc(function, instance))
        , instance_(instance)
    {
    }

    vfunc(function_type function, object_type* instance, std::in_place_t)
        : func_(function)
        , instance_(instance)
    {
    }

    template <std::same_as<Fn> T>
    operator T() const
    {
        return func_;
    }

    template <typename... Args>
    auto operator()(Args&&... args) const noexcept(info::no_throw)
#ifdef _DEBUG
        -> std::invoke_result_t<Fn, object_type*, Args&&...>
#else
        -> return_type
#endif
    {
        return std::invoke(func_, instance_, std::forward<Args>(args)...);
    }

    /*function_type get_full() const
    {
        return unsafe_cast<function_type>(basic_vfunc<Object>::get());
    }*/

    /*Ret operator()(Args... args) const
    {
        member_func_invoker<Call_T, Ret, T, Args...> invoker;
        return invoker(function_, instance_, args...);
    }*/
};

template <typename Func, class Object>
vfunc(Func fn, Object instance) -> vfunc<Func>;

template <typename Func, class Object>
vfunc(Func fn, Object instance, std::in_place_t) -> vfunc<Func>;
} // namespace fd