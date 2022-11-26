#include <fd/assert.h>
#include <fd/filesystem.h>

#include <Shlwapi.h>
#include <Windows.h>

#pragma comment(lib, "Shlwapi.lib")

using namespace fd;
using namespace fs;

template <class T>
static bool _Is_null_terminated(const T str)
{
    const auto raw = str.data();
    return raw[str.size()] == '\0';
}

static bool _Is_directory(const DWORD attr)
{
    return attr != INVALID_FILE_ATTRIBUTES && attr & FILE_ATTRIBUTE_DIRECTORY;
}

template <class T>
static bool _Is_nt_string(const T str)
{
    return str[0] == '\\' && str[1] == '?';
}

template <class T>
static bool _Is_short_path(const T str)
{
    return str.size() < MAX_PATH;
}

template <class T>
constexpr bool _Is_wstring(const basic_string_view<T>)
{
    return std::same_as<T, wchar_t>;
}

#define _FIX_LONG_PATH(_PATH_) make_string(L"\\?", _PATH_).data()
#define _FIX_WIDE(_PATH_)      (_Is_wstring(_PATH_) ? reinterpret_cast<const wchar_t*>(_PATH_.data()) : wstring(_PATH_.begin(), _PATH_.end()).data())

#define FIX_C_STR(_STR_)      (_Is_null_terminated(_STR_) ? _STR_.data() : basic_string(_STR_.begin(), _STR_.end()).data())
#define FIX_LONG_PATH(_PATH_) (_Is_nt_string(_PATH_) ? _FIX_WIDE(_PATH_) : _FIX_LONG_PATH(_PATH_))
#define FIX_C_PATH(_PATH_)    (_Is_nt_string(_PATH_) || _Is_short_path(_PATH_) ? FIX_C_STR(_PATH_) : _FIX_LONG_PATH(_PATH_))

bool directory_impl::operator()(const string_view dir) const
{
    const auto short_path = _Is_short_path(dir);
#ifdef PathIsDirectory
    return short_path ? PathIsDirectoryA(FIX_C_STR(dir)) : PathIsDirectoryW(FIX_LONG_PATH(dir));
#else
    const auto attr = short_path ? GetFileAttributesA(FIX_C_STR(dir)) : GetFileAttributesW(FIX_LONG_PATH(dir));
    return _Is_directory(attr);
#endif
}

bool directory_impl::operator()(const wstring_view dir) const
{
#ifdef PathIsDirectory
    return PathIsDirectoryW(FIX_C_PATH(dir));
#else
    return _Is_directory(GetFileAttributesW(FIX_C_PATH(dir)));
#endif
}

bool directory_impl::create(const string_view dir) const
{
    return _Is_short_path(dir) ? CreateDirectoryA(FIX_C_STR(dir), nullptr) : CreateDirectoryW(FIX_LONG_PATH(dir), nullptr);
}

bool directory_impl::create(const wstring_view dir) const
{
    return CreateDirectoryW(FIX_C_PATH(dir), nullptr);
}

bool directory_impl::empty(const string_view dir) const
{
#ifdef PathIsDirectoryEmpty
    return _Is_short_path(dir) ? PathIsDirectoryEmptyA(FIX_C_STR(dir)) : PathIsDirectoryEmptyW(FIX_LONG_PATH(dir));
#else

#endif
}

bool directory_impl::empty(const wstring_view dir) const
{
#ifdef PathIsDirectoryEmpty
    return PathIsDirectoryEmptyW(FIX_C_PATH(dir));
#else

#endif
}

//-----

bool file_impl::operator()(const string_view dir) const
{
    const auto attr = _Is_short_path(dir) ? GetFileAttributesA(FIX_C_STR(dir)) : GetFileAttributesW(FIX_LONG_PATH(dir));
    return attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY);
}

bool file_impl::operator()(const wstring_view dir) const
{
    const auto attr = GetFileAttributesW(FIX_C_PATH(dir));
    return attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY);
}