#if 0
#include "functional/ignore.h"
#include "debug/console.h"

#include <boost/nowide/iostream.hpp>

#include <tchar.h>

#include <cassert>
#include <iostream>

namespace fd
{
system_console::~system_console()
{
    SetStdHandle(STD_ERROR_HANDLE, old_err_w);
    SetStdHandle(STD_INPUT_HANDLE, old_in_w);
    SetStdHandle(STD_OUTPUT_HANDLE, old_out_w);

    CloseHandle(in_w);
    CloseHandle(out_w);

    fclose(err);
    fclose(in);
    fclose(out);

    FreeConsole();
}

system_console::system_console()
{
    
    if (!AllocConsole())
    {
        if (exists())
            return;
        // Add some error handling here.
        // You can call GetLastError() to get more info about the error.
        assert(0 && "Unable to allocate console!");
    }

    // std::cout, std::clog, std::cerr, std::cin
    freopen_s(&out, "CONOUT$", "w", stdout);
    freopen_s(&err, "CONOUT$", "w", stderr);
    freopen_s(&in, "CONIN$", "r", stdin);

    old_out_w = GetStdHandle(STD_OUTPUT_HANDLE);
    old_err_w = GetStdHandle(STD_ERROR_HANDLE);
    old_in_w  = GetStdHandle(STD_INPUT_HANDLE);

    // std::wcout, std::wclog, std::wcerr, std::wcin
    out_w = CreateFile(_T("CONOUT$"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    in_w  = CreateFile(_T("CONIN$"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (!SetStdHandle(STD_OUTPUT_HANDLE, out_w))
        assert(0 && "Unable to set output handle!");
    if (!SetStdHandle(STD_ERROR_HANDLE, out_w))
        assert(0 && "Unable to set error handle!");
    if (!SetStdHandle(STD_INPUT_HANDLE, in_w))
        assert(0 && "Unable to set input handle!");

    std::cout.clear();
    std::clog.clear();
    std::cerr.clear();
    std::cin.clear();

    std::wcout.clear();
    std::wclog.clear();
    std::wcerr.clear();
    std::wcin.clear();

#define BOOST_NO_WIDE_RECREATE_EX(_T_, _DEF_, ...)        \
    if (boost::nowide::_T_.rdbuf() == std::_DEF_.rdbuf()) \
    {                                                     \
        using namespace boost::nowide;                    \
        std::destroy_at(&_T_);                            \
        std::construct_at(&_T_, __VA_ARGS__);             \
    }
#define BOOST_NO_WIDE_RECREATE(_T_, ...) BOOST_NO_WIDE_RECREATE_EX(_T_, _T_, __VA_ARGS__)

    BOOST_NO_WIDE_RECREATE(cout, true, nullptr);
    BOOST_NO_WIDE_RECREATE(cin, &cout);
    BOOST_NO_WIDE_RECREATE(cerr, false, &cout);
    BOOST_NO_WIDE_RECREATE_EX(clog, cerr, false, nullptr);

    /*boost::nowide::cout.clear();
    boost::nowide::clog.clear();
    boost::nowide::cerr.clear();
    boost::nowide::cin.clear();*/
}

bool system_console::exists() const
{
    ignore_unused(this);
    return GetConsoleWindow();
}

} // namespace fd_NAMESPACE
#endif