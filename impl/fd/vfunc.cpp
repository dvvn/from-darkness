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
struct vfunc_index_getter
{
    size_t operator()(void *function, void *instance, size_t vtable_offset) const
    {
        vfunc_index_resolver<Call> resolver;
        auto backup = make_mem_backup(get_vtable(instance)[vtable_offset], get_vtable(&resolver)[0]);
        return member_func_builder<Call, size_t, void>::invoke(function, instance);
    }
};

#define GET_INDEX_FN(z, i, __call) \
    virtual size_t __call i_##i()  \
    {                              \
        return i;                  \
    }

#define GENERATE_CALL(call__, __call, _call_)                         \
    template <>                                                       \
    struct vfunc_index_resolver<call__>                               \
    {                                                                 \
        BOOST_PP_REPEAT(BOOST_PP_LIMIT_REPEAT, GET_INDEX_FN, __call); \
    };

X86_CALL_MEMBER(GENERATE_CALL);
#undef GENERATE_CALL
#undef GET_INDEX_FN

size_t get_vfunc_index(call_type_t call, void *function, void *instance, size_t vtable_offset)
{
    return apply<vfunc_index_getter>(call, function, instance, vtable_offset);
}
}