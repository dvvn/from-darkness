#pragma once

#include <fd/core/utility.h>

#ifdef _DEBUG
import fd.assert;

#define _FD_ASSERT(_EXPRESSION_OR_MESSAGE_, ...)                \
    if (fd::can_invoke_assert_handler(_EXPRESSION_OR_MESSAGE_)) \
        std::invoke(fd::assert_handler, fd::assert_data(FD_STRINGIZE(_EXPRESSION_OR_MESSAGE_), ##__VA_ARGS__));
#define FD_ASSERT(...)           \
    {                            \
        _FD_ASSERT(__VA_ARGS__); \
    }
#define FD_ASSERT_UNREACHABLE(_MESSAGE_) \
    {                                    \
        _FD_ASSERT(_MESSAGE_);           \
        fd::unreachable();               \
    }
#else
#define FD_ASSERT(...)             (void)0
#define FD_ASSERT_UNREACHABLE(...) fd::unreachable()
#endif
