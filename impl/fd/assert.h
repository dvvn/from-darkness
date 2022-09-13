#pragma once

#include <fd/utility.h>

#ifdef _DEBUG
import fd.assert;

#define FD_ASSERT(_EXPRESSION_OR_MESSAGE_, /*message for expression*/...)                                                               \
    {                                                                                                                                   \
        fd::invoke(fd::assert_handler, fd::assert_data(FD_STRINGIZE(_EXPRESSION_OR_MESSAGE_), ##__VA_ARGS__), _EXPRESSION_OR_MESSAGE_); \
    }
#define FD_ASSERT_UNREACHABLE(_MESSAGE_)                            \
    {                                                               \
        fd::invoke(fd::assert_handler, fd::assert_data(_MESSAGE_)); \
        fd::unreachable();                                          \
    }
#else
#define FD_ASSERT(...)             (void)0;
#define FD_ASSERT_UNREACHABLE(...) fd::unreachable();
#endif
