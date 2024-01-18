#pragma once



#include <functional>

// bind to function pointer hurt perfomance and made code looks ugly
#ifdef _MSC_VER
#define _BIND_BLOCKER(_CCV_, _CV_, _REF_, _NOEXCEPT_)                                            \
    template <typename RetSpec, typename Ret, class Obj, typename... Args, typename... ArgsSpec> \
    class _Binder<RetSpec, Ret (_CCV_ Obj::*)(Args...) _CV_ _REF_ _NOEXCEPT_, ArgsSpec...>       \
    {                                                                                            \
      public:                                                                                    \
        ~_Binder() = delete;                                                                     \
    };                                                                                           \
    template <typename Ret, class Obj, typename... Args, typename... ArgsSpec>                   \
    class _Front_binder<Ret (_CCV_ Obj::*)(Args...) _CV_ _REF_ _NOEXCEPT_, ArgsSpec...>          \
    {                                                                                            \
      public:                                                                                    \
        ~_Front_binder() = delete;                                                               \
    };                                                                                           \
    template <typename Ret, class Obj, typename... Args, typename... ArgsSpec>                   \
    class _Back_binder<Ret (_CCV_ Obj::*)(Args...) _CV_ _REF_ _NOEXCEPT_, ArgsSpec...>           \
    {                                                                                            \
      public:                                                                                    \
        ~_Back_binder() = delete;                                                                \
    };                                                                                           \
                                                                                                 \
    template <typename RetSpec = void, typename Ret, class Obj, typename... Args>                \
    auto bind(Ret (_CCV_ Obj::*)(Args...) _CV_ _REF_ _NOEXCEPT_, auto&&...) = delete;            \
    template <typename Ret, class Obj, typename... Args>                                         \
    auto bind_front(Ret (_CCV_ Obj::*)(Args...) _CV_ _REF_ _NOEXCEPT_, auto&&...) = delete;      \
    template <typename Ret, class Obj, typename... Args>                                         \
    auto bind_back(Ret (_CCV_ Obj::*)(Args...) _CV_ _REF_ _NOEXCEPT_, auto&&...) = delete;

_STD_BEGIN
_MEMBER_CALL_CV_REF_NOEXCEPT(_BIND_BLOCKER);
_STD_END

#undef _BIND_BLOCKER
#else
namespace fd
{
template <typename Fn>
requires(std::is_member_function_pointer_v<Fn>)
auto bind_back(Fn, auto&&...) = delete;

template <typename Fn>
requires(std::is_member_function_pointer_v<Fn>)
auto bind_front(Fn, auto&&...) = delete;

template <typename Fn>
requires(std::is_member_function_pointer_v<Fn>)
auto bind(Fn, auto&&...) = delete;
} // namespace fd
#endif

namespace fd
{
using std::bind;
using std::bind_back;
using std::bind_front;
} // namespace fd
