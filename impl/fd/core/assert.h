#pragma once

#ifdef _DEBUG
#include <fd/core/utility.h>
import fd.assert;
// import fd.one_instance;//for std::invoke wrapper

#define FD_ASSERT(_EXPRESSION_OR_MESSAGE_, ...)                                                             \
    {                                                                                                       \
        using namespace fd;                                                                                 \
        if (can_invoke_assert_handler(_EXPRESSION_OR_MESSAGE_))                                             \
        {                                                                                                   \
            std::invoke(assert_handler, assert_data(FD_STRINGIZE(_EXPRESSION_OR_MESSAGE_), ##__VA_ARGS__)); \
            unreachable();                                                                                  \
        }                                                                                                   \
    }

//#define fds_assert_add_handler(_HANDLER_)   fd::_Rt_assert_add(_HANDLER_)
//#define fds_assert_remove_handle(_HANDLER_) fd::_Rt_assert_remove(_HANDLER_)
#define FD_ASSERT_UNREACHABLE FD_ASSERT
#else
#include <fd/core/utility.h>
#define FD_ASSERT(...)             (void)0
//#define fds_assert_add_handler(...)    (void)0
//#define fds_assert_remove_handler(...) (void)0
#define FD_ASSERT_UNREACHABLE(...) fd::unreachable()
#endif
