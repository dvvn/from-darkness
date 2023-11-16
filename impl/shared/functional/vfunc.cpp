#include "functional/vfunc.h"

#ifdef __RESHARPER__
#define BOOST_PP_LIMIT_REPEAT 1
#endif

#include <boost/preprocessor/repeat.hpp>

namespace fd
{
template <class Call_T>
struct vfunc_index_resolver;

template <class Call_T>
static size_t get_vfunc_index(void* function) noexcept
{
    using resolver_type = vfunc_index_resolver<Call_T>;
    using function_type = typename member_function<true, Call_T, size_t, resolver_type const>::type;

    resolver_type resolver;
    return std::invoke(unsafe_cast<function_type>(function), &resolver);
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

void* get_vfunc(size_t const index, void* instance)
{
    return get_vtable(instance)[index];
}

#define GET_VFUNC_IMPL(_CCV_, ...)                                       \
    template <>                                                          \
    void* get_vfunc<_CCV_T(_CCV_)>(void* table_function, void* instance) \
    {                                                                    \
        return get_vfunc_impl<_CCV_T(_CCV_)>(table_function, instance);  \
    }

#define GET_INDEX_FN(_Z_UNUSED_, _INDEX_, _CCV_)        \
    virtual size_t _CCV_ get_##_INDEX_() const noexcept \
    {                                                   \
        return _INDEX_;                                 \
    }

#define VFUNC_INDEX_RESOLVER(_CCV_, ...)                             \
    template <>                                                      \
    struct vfunc_index_resolver<_CCV_T(_CCV_)>                       \
    {                                                                \
        BOOST_PP_REPEAT(BOOST_PP_LIMIT_REPEAT, GET_INDEX_FN, _CCV_); \
    };

#ifdef _MSC_VER
_MEMBER_CALL(GET_VFUNC_IMPL, , , )
_MEMBER_CALL(VFUNC_INDEX_RESOLVER, , , ) // NOLINT(cppcoreguidelines-virtual-class-destructor, clang-diagnostic-extra-semi)
#else

#endif

#undef GET_VFUNC_IMPL
#undef GET_INDEX_FN
#undef VFUNC_INDEX_RESOLVER
}