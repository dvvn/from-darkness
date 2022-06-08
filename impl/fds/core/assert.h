#pragma once

#ifdef _DEBUG
#include <fds/core/assert_impl.h>
#define fds_assert(_EXPRESSION_OR_MESSAGE_, ...)                                           \
    {                                                                                      \
        if (fds::_Rt_assert_can_invoke(_EXPRESSION_OR_MESSAGE_))                           \
            fds::_Rt_assert_invoke(FDS_STRINGIZE(_EXPRESSION_OR_MESSAGE_), ##__VA_ARGS__); \
    }

#define fds_assert_add_handler(_HANDLER_)   fds::_Rt_assert_add(_HANDLER_)
#define fds_assert_remove_handle(_HANDLER_) fds::_Rt_assert_remove(_HANDLER_)
#define fds_assert_unreachable              fds_assert
#else
#include <fds/core/utility.h>
#define fds_assert(...)                (void)0
#define fds_assert_add_handler(...)    (void)0
#define fds_assert_remove_handler(...) (void)0
#define fds_assert_unreachable(...)    fds::unreachable()
#endif
