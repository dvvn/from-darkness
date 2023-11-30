#pragma once

#include "tier0/core.h"

#include <Windows.h>

#include <cstdio>

namespace FD_TIER(2)::inline debug
{
class system_console
{
    FILE* in;

    FILE* out;
    FILE* err;

    HANDLE in_w;
    HANDLE out_w;

    HANDLE old_out_w;
    HANDLE old_in_w;
    HANDLE old_err_w;

  public:
    ~system_console();
    system_console();

    bool exists() const;
};
}