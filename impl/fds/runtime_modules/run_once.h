#pragma once

#include <fds/core/assert.h>

template <auto... Args>
void _Run_once(const char* func_sig)
{
    [[maybe_unused]] static auto func_sig_stored = func_sig;
    fds_assert(func_sig_stored == func_sig, "Already called with different template args");
}

#define FDS_RTM_RUN_ONCE(...) _Run_once<__VA_ARGS__>(__FUNCSIG__);
