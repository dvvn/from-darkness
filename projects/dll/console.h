#pragma once

#include <cstdio>

#include <Windows.h>

namespace fd
{
class console_holder
{
    FILE* in;
    FILE* out;
    FILE* err;

    HANDLE inW;
    HANDLE outW;

    HANDLE oldOutW;
    HANDLE oldInW;
    HANDLE oldErrW;

  public:
    ~console_holder();
    console_holder(wchar_t const* title);
};
}