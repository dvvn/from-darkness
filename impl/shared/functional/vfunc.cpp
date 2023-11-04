#include "functional/vfunc.h"

#include <boost/preprocessor/repeat.hpp>

// for buggy resharper
#if !defined(BOOST_PP_LIMIT_REPEAT) || defined(__RESHARPER__)
#define BOOST_PP_LIMIT_REPEAT 1
#endif

namespace fd
{
template <class Call_T>
struct vfunc_index_resolver;

template <class Call_T>
static size_t get_vfunc_index(void* function) noexcept
{
    member_func_invoker<Call_T, size_t, void> invoker;
    vfunc_index_resolver<Call_T> resolver;
    return invoker(function, &resolver);
}

static void** get_vtable(void* instance)
{
    return *static_cast<void***>(instance);
}

template <class Call_T>
static void* get_vfunc_impl(void* table_function, void* instance)
{
    auto const function_index = get_vfunc_index<Call_T>(table_function);
    return get_vtable(instance)[function_index];
}

#define GET_VFUNC_IMPL(call__, __call, _call_)                    \
    template <>                                                   \
    void* get_vfunc<call__>(void* table_function, void* instance) \
    {                                                             \
        return get_vfunc_impl<call__>(table_function, instance);  \
    }

X86_CALL_MEMBER(GET_VFUNC_IMPL);
#undef GET_VFUNC_IMPL

#define GET_INDEX_FN(z, i, __call) \
    virtual size_t __call i_##i()  \
    {                              \
        return i;                  \
    }

#define VFUNC_INDEX_RESOLVER(call__, __call, _call_)                  \
    template <>                                                       \
    struct vfunc_index_resolver<call__> final                         \
    {                                                                 \
        BOOST_PP_REPEAT(BOOST_PP_LIMIT_REPEAT, GET_INDEX_FN, __call); \
    };
X86_CALL_MEMBER(VFUNC_INDEX_RESOLVER); // NOLINT(cppcoreguidelines-virtual-class-destructor, clang-diagnostic-extra-semi)
}