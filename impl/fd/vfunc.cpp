#include "mem_backup.h"
#include "vfunc.h"

#include <boost/preprocessor/repeat.hpp>

// for buggy resharper
#ifndef BOOST_PP_LIMIT_REPEAT
#define BOOST_PP_LIMIT_REPEAT 1
#endif

namespace fd
{
template <call_type_t Call>
struct vfunc_index_resolver;

template <call_type_t Call>
static size_t get_vfunc_index(void *function, void *instance) noexcept
{
    vfunc_index_resolver<Call> resolver;
    member_func_invoker<Call, size_t, void> invoker;
    auto backup = make_mem_backup(get_vtable_ref(instance), get_vtable(&resolver));
    return invoker(function, instance);
}

#define GET_VFUNC_IDX_IMPL(call__, __call, _call_)                                   \
    size_t get_vfunc_index(call_type_holder<call__>, void *function, void *instance) \
    {                                                                                \
        return get_vfunc_index<call__>(function, instance);                          \
    }

X86_CALL_MEMBER(GET_VFUNC_IDX_IMPL);
#undef GET_VFUNC_IDX_IMPL

template <call_type_t Call>
using vfunc_index_getter_base = decltype(bind_front(get_vfunc_index<Call>));

template <call_type_t Call>
struct vfunc_index_getter : vfunc_index_getter_base<Call>
{
    vfunc_index_getter()
        : vfunc_index_getter_base<Call>(get_vfunc_index<Call>)
    {
    }
};

size_t get_vfunc_index(call_type_t call, void *function, void *instance)
{
    return apply<vfunc_index_getter>(call, function, instance);
}

#define GET_INDEX_FN(z, i, __call) \
    virtual size_t __call i_##i()  \
    {                              \
        return i;                  \
    }

#define VFUNC_INDEX_RESOLVER(call__, __call, _call_)                  \
    template <>                                                       \
    struct vfunc_index_resolver<call__>                               \
    {                                                                 \
        BOOST_PP_REPEAT(BOOST_PP_LIMIT_REPEAT, GET_INDEX_FN, __call); \
    };
X86_CALL_MEMBER(VFUNC_INDEX_RESOLVER); // NOLINT(cppcoreguidelines-virtual-class-destructor)
}