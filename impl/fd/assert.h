#pragma once

#ifdef _DEBUG
#include <fd/assert_handler.h>

#define FD_ASSERT(_EXPRESSION_OR_MESSAGE_, /*message for expression*/...) \
    /**/                                                                  \
    run_assert({ #_EXPRESSION_OR_MESSAGE_, ##__VA_ARGS__ }, _EXPRESSION_OR_MESSAGE_);

#define FD_ASSERT_PANIC(_MESSAGE_) \
    /**/                           \
    run_panic_assert({ _MESSAGE_ });

#else
#define FD_ASSERT(...)       (void)0;
#define FD_ASSERT_PANIC(...) throw;
#endif