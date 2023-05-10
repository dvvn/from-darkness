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

#define GENERATE_CALL(call__, __call)                                   \
    static struct                                                       \
    {                                                                   \
        BOOST_PP_REPEAT(BOOST_PP_LIMIT_REPEAT, VTIC_GET_INDEX, __call); \
    } __call##_calculator;

X86_CALL_MEMBER(GENERATE_CALL);
#undef GENERATE_CALL
#undef VTIC_GET_INDEX

#define GENERATE_VTABLES(call__, __call) &__call##_calculator,

static vtable<void> call_vtable[] = {
    //
    X86_CALL_MEMBER(GENERATE_VTABLES)};
#undef GENERATE_VTABLES

size_t get_vfunc_index(void *instance, size_t vtable_offset, void *function, _x86_call call)
{
    using num_t = std::underlying_type_t<_x86_call>;

    vtable vt   = instance;
    auto backup = exchange(vt, call_vtable[static_cast<num_t>(call)]);

    auto index = unknown_vfunc_invoker<>::call<size_t>(instance, function, call);

    vt.set(backup);

    return index;
}
}