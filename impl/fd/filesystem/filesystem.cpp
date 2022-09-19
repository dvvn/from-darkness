module;

#include <Windows.h>

#include <fd/assert.h>

module fd.filesystem;
import fd.functional.invoke;

using namespace fd;
using namespace fs;

template <class T>
concept have_fn_operator = requires { T::operator(); };

struct invocable_gap
{
    void operator()(int*) const
    {
    }
};

template <funcs Tag>
struct tagged_fn;

template <funcs Tag, class Base = std::conditional_t<have_fn_operator<tagged_fn<Tag>>, tagged_fn<Tag>, invocable_gap>>
struct tagged_fn_ex : Base
{
    using Base::operator();

    bool operator()(const char8_t* ptr) const requires(!invocable<Base, const char8_t*>)
    {
        FD_ASSERT_UNREACHABLE("Not implemented");
    }

    bool operator()(const char* ptr) const requires(!invocable<Base, const char*>)
    {
        FD_ASSERT_UNREACHABLE("Not implemented");
    }

    bool operator()(const wchar_t* ptr) const requires(!invocable<Base, const wchar_t*>)
    {
        FD_ASSERT_UNREACHABLE("Not implemented");
    }

    bool operator()(const char16_t* ptr) const requires(!invocable<Base, const char16_t*>)
    {
        if constexpr (sizeof(char16_t) == sizeof(wchar_t))
            return operator()(reinterpret_cast<const wchar_t*>(ptr));
        else
            FD_ASSERT_UNREACHABLE("Not implemented");
    }

    bool operator()(const char32_t* ptr) const requires(!invocable<Base, const char32_t*>)
    {
        if constexpr (sizeof(char32_t) == sizeof(wchar_t))
            return operator()(reinterpret_cast<const wchar_t*>(ptr));
        else
            FD_ASSERT_UNREACHABLE("Not implemented");
    }
};

template <>
struct tagged_fn<funcs::dir_create>
{
    bool operator()(const char* dir) const
    {
        return CreateDirectoryA(dir, nullptr);
    }

    bool operator()(const wchar_t* dir) const
    {
        return CreateDirectoryW(dir, nullptr);
    }
};

template <>
struct tagged_fn<funcs::is_directory>
{
    bool operator()(const char* dir) const
    {
        const auto attr = GetFileAttributesA(dir);
        return attr != INVALID_FILE_ATTRIBUTES && attr & FILE_ATTRIBUTE_DIRECTORY;
    }

    bool operator()(const wchar_t* dir) const
    {
        const auto attr = GetFileAttributesW(dir);
        return attr != INVALID_FILE_ATTRIBUTES && attr & FILE_ATTRIBUTE_DIRECTORY;
    }
};

template <>
struct tagged_fn<funcs::is_file>
{
    bool operator()(const char* dir) const
    {
        const auto attr = GetFileAttributesA(dir);
        return attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY);
    }

    bool operator()(const wchar_t* dir) const
    {
        const auto attr = GetFileAttributesW(dir);
        return attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY);
    }
};

// WIN32_FILE_ATTRIBUTE_DATA
// FindFirstFile

static bool _Check_close_find_handle(HANDLE h)
{
    const auto ok = h != INVALID_HANDLE_VALUE;
    if (ok)
        FindClose(h);
    return ok;
}

template <>
struct tagged_fn<funcs::dir_empty>
{
    [[no_unique_address]] tagged_fn<funcs::is_directory> dir_exists_;

    bool operator()(const char* dir) const
    {
        return dir_exists_(dir) && !_Check_close_find_handle(FindFirstFileA(dir, nullptr));
    }

    bool operator()(const wchar_t* dir) const
    {
        return dir_exists_(dir) && !_Check_close_find_handle(FindFirstFileW(dir, nullptr));
    }
};

template <>
struct tagged_fn<funcs::file_empty>
{
    //[[no_unique_address]] tagged_fn<funcs::file_exists> file_exists_;

    /* bool operator()(const char* dir) const
    {
    }

    bool operator()(const wchar_t* dir) const
    {
    } */
};

#define INVOKE(_TAG_)  \
    case funcs::_TAG_: \
        return invoke(tagged_fn_ex<funcs::_TAG_>(), args...);

#undef error

template <typename... Args>
static auto invoke_fn(const funcs tag, Args... args)
{
    switch (tag)
    {
        INVOKE(dir_create);
        // INVOKE(file_create);
        INVOKE(is_directory);
        INVOKE(is_file);
        INVOKE(dir_empty);
        INVOKE(file_empty);
    default:
        FD_ASSERT_UNREACHABLE("Incorrect tag");
    };
}

bool basic_function_selector::operator()(const char8_t* str) const
{
    return invoke_fn(fn_tag_, str);
}

bool basic_function_selector::operator()(const char* str) const
{
    return invoke_fn(fn_tag_, str);
}

bool basic_function_selector::operator()(const wchar_t* str) const
{
    return invoke_fn(fn_tag_, str);
}

bool basic_function_selector::operator()(const char16_t* str) const
{
    return invoke_fn(fn_tag_, str);
}

bool basic_function_selector::operator()(const char32_t* str) const
{
    return invoke_fn(fn_tag_, str);
}

//----

template <typename... Args>
static auto invoke_fn(bool& error, const funcs tag, Args... args)
{
    const auto result = invoke_fn(tag, args...);
    if (result)
        error = false;
    else
        error = GetLastError() != 0;
    return result;
}

function_selector::function_selector(const funcs tag)
    : fn_tag_(tag)
    , error_(false)
{
}

bool function_selector::operator()(const char8_t* str)
{
    return invoke_fn(error_, fn_tag_, str);
}

bool function_selector::operator()(const char* str)
{
    return invoke_fn(error_, fn_tag_, str);
}

bool function_selector::operator()(const wchar_t* str)
{
    return invoke_fn(error_, fn_tag_, str);
}

bool function_selector::operator()(const char16_t* str)
{
    return invoke_fn(error_, fn_tag_, str);
}

bool function_selector::operator()(const char32_t* str)
{
    return invoke_fn(error_, fn_tag_, str);
}

function_selector::operator bool() const
{
    return error_;
}
