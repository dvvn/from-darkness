#pragma once

#include <type_traits>

#ifdef __cpp_lib_unreachable
#include <utility>
#define _FD_UNREACHABLE std::unreachable
#else
#include <exception>
#define _FD_UNREACHABLE std::terminate
#endif

#ifdef _DEBUG
#include <fd/assert_handler.h>
#include <fd/utility.h>

#define FD_ASSERT(_EXPRESSION_OR_MESSAGE_, /*message for expression*/...) \
    /**/                                                                  \
    invoke(AssertHandler, assert_data(#_EXPRESSION_OR_MESSAGE_, ##__VA_ARGS__), _EXPRESSION_OR_MESSAGE_);

#define FD_ASSERT_UNREACHABLE(_MESSAGE_) \
    /**/                                 \
    invoke(AssertHandler, assert_data(_MESSAGE_), _FD_UNREACHABLE);
#else
#define FD_ASSERT(...)             (void)0;
#define FD_ASSERT_UNREACHABLE(...) _FD_UNREACHABLE();
#endif
