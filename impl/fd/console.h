#pragma once
#include <Windows.h>

#include <cstdio>

namespace fd
{
class system_console
{
    FILE *in;

    FILE *out;
    FILE *err;

    HANDLE in_w;
    HANDLE out_w;

    HANDLE old_out_w;
    HANDLE old_in_w;
    HANDLE old_err_w;

  public:
    ~system_console();
    system_console();

    bool init();
};
}