#include "vfunc.h"

#include <boost/preprocessor/repeat.hpp>

// for buggy resharper
#ifndef BOOST_PP_LIMIT_REPEAT
#define BOOST_PP_LIMIT_REPEAT 1
#endif

namespace fd
{
#define VTIC_GET_INDEX(z, i, __call) \
    virtual size_t __call i_##i()    \
    {                                \
        return i;                    \
    }

#define GENERATE_CALL(call__, __call, call)                             \
    static struct                                                       \
    {                                                                   \
        BOOST_PP_REPEAT(BOOST_PP_LIMIT_REPEAT, VTIC_GET_INDEX, __call); \
    } call##_calculator;

X86_CALL_MEMBER(GENERATE_CALL);
#undef GENERATE_CALL
#undef VTIC_GET_INDEX

#define GENERATE_VTABLES(call__, __call, call) &call##_calculator,

static vtable<void> call_vtable[] = {
    X86_CALL_MEMBER(GENERATE_VTABLES) //
};
#undef GENERATE_VTABLES

size_t get_vfunc_index(void *instance, size_t vtable_offset, void *function, call_type_t call)
{
    using num_t = std::underlying_type_t<call_type_t>;

    auto vt     = vtable(instance, vtable_offset);
    auto backup = vt.replace(call_vtable[static_cast<num_t>(call)]);

    return member_func_invoker<call_type_t::unknown, size_t>::call(instance, function, call);
}
}