#pragma once

#ifdef _DEBUG
#include <fds/core/assert_impl.h>
#define fds_assert                fds_assert_call
#define fds_assert_add_handler    fds_assert_add_handler_impl
#define fds_assert_remove_handler fds_assert_remove_handler_impl
#define fds_assert_unreachable    fds_assert_call
#else
#include <fds/core/utility.h>
#define fds_assert(...)                (void)0
#define fds_assert_add_handler(...)    (void)0
#define fds_assert_remove_handler(...) (void)0
#define fds_assert_unreachable(...)    fds::unreachable()
#endif
