#include <tchar.h>
#include <windows.h>

#include <cstdio>
#include <iostream>

class console_holder
{
    FILE *in;
    FILE *out;
    FILE *err;

    HANDLE inW;
    HANDLE outW;

    HANDLE oldOutW;
    HANDLE oldInW;
    HANDLE oldErrW;

  public:
    ~console_holder()
    {
        SetStdHandle(STD_ERROR_HANDLE, oldErrW);
        SetStdHandle(STD_INPUT_HANDLE, oldInW);
        SetStdHandle(STD_OUTPUT_HANDLE, oldOutW);

        CloseHandle(inW);
        CloseHandle(outW);

        fclose(err);
        fclose(in);
        fclose(out);

        FreeConsole();
    }

    console_holder()
    {
        if (!AllocConsole())
        {
            // Add some error handling here.
            // You can call GetLastError() to get more info about the error.
            return;
        }

        // std::cout, std::clog, std::cerr, std::cin
        freopen_s(&out, "CONOUT$", "w", stdout);
        freopen_s(&err, "CONOUT$", "w", stderr);
        freopen_s(&in, "CONIN$", "r", stdin);
        std::cout.clear();
        std::clog.clear();
        std::cerr.clear();
        std::cin.clear();

        oldOutW = GetStdHandle(STD_OUTPUT_HANDLE);
        oldErrW = GetStdHandle(STD_ERROR_HANDLE);
        oldInW  = GetStdHandle(STD_INPUT_HANDLE);

        // std::wcout, std::wclog, std::wcerr, std::wcin
        outW = CreateFile(
            _T("CONOUT$"),
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);
        inW = CreateFile(
            _T("CONIN$"),
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);
        if (!SetStdHandle(STD_OUTPUT_HANDLE, outW))
            return;
        if (!SetStdHandle(STD_ERROR_HANDLE, outW))
            return;
        if (!SetStdHandle(STD_INPUT_HANDLE, inW))
            return;
        std::wcout.clear();
        std::wclog.clear();
        std::wcerr.clear();
        std::wcin.clear();
    }
};