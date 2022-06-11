#pragma once

#ifdef _DEBUG
#include <fds/core/utility.h>
import fds.assert;
// import fds.one_instance;//for std::invoke wrapper

#define FDS_ASSERT(_EXPRESSION_OR_MESSAGE_, ...)                                                             \
    {                                                                                                        \
        using namespace fds;                                                                                 \
        if (can_invoke_assert_handler(_EXPRESSION_OR_MESSAGE_))                                              \
        {                                                                                                    \
            std::invoke(assert_handler, assert_data(FDS_STRINGIZE(_EXPRESSION_OR_MESSAGE_), ##__VA_ARGS__)); \
            unreachable();                                                                                   \
        }                                                                                                    \
    }

//#define fds_assert_add_handler(_HANDLER_)   fds::_Rt_assert_add(_HANDLER_)
//#define fds_assert_remove_handle(_HANDLER_) fds::_Rt_assert_remove(_HANDLER_)
#define FDS_ASSERT_UNREACHABLE FDS_ASSERT
#else
#include <fds/core/utility.h>
#define FDS_ASSERT(...)             (void)0
//#define fds_assert_add_handler(...)    (void)0
//#define fds_assert_remove_handler(...) (void)0
#define FDS_ASSERT_UNREACHABLE(...) fds::unreachable()
#endif
