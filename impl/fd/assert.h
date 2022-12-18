#pragma once

#include <fd/exception.h>

#ifdef _DEBUG
#include <fd/assert_handler.h>

#define FD_ASSERT(_EXPRESSION_OR_MESSAGE_, /*message for expression*/...) \
    /**/                                                                  \
    invoke(AssertHandler, assert_data(#_EXPRESSION_OR_MESSAGE_, ##__VA_ARGS__), _EXPRESSION_OR_MESSAGE_);

#define FD_ASSERT_UNREACHABLE(_MESSAGE_) \
    /**/                                 \
    invoke(AssertHandler, assert_data(_MESSAGE_), unreachable);
#else
#define FD_ASSERT(...) (void)0;
#define FD_ASSERT_UNREACHABLE(...) unreachable();
#endif