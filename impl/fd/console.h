#pragma once
#include <Windows.h>

#include <cstdio>

namespace fd
{
bool create_system_console();
void destroy_system_console();

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