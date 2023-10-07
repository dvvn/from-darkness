#undef UNICODE
#include "system_error.h"
#include "container/array.h"
#include "container/vector/dynamic.h"
#include "iterator/unwrap.h"

#include <comdef.h>

#include <algorithm>
#include <memory>

// ReSharper disable CppInconsistentNaming
static BOOL WIN32_FROM_HRESULT(HRESULT hr, OUT DWORD* pdwWin32)
{
    if ((hr & 0xFFFF0000) == MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, 0))
    {
        // Could have come from many values, but we choose this one
        *pdwWin32 = HRESULT_CODE(hr);
        return TRUE;
    }
    if (hr == S_OK)
    {
        *pdwWin32 = HRESULT_CODE(hr);
        return TRUE;
    }
    // otherwise, we got an impossible value
    return FALSE;
}

// ReSharper restore CppInconsistentNaming

namespace fd
{
static char const* format_hresult(HRESULT result)
{
    uint8_t buffer[sizeof(_com_error)];
    auto err = new (buffer) _com_error(result);
    return err->ErrorMessage();
}

struct local_free_wrapped
{
    void operator()(char const* ptr) const noexcept
    {
        LocalFree((HLOCAL)ptr);
    }
};

struct error_string : std::unique_ptr<TCHAR const, local_free_wrapped>
{
    error_string() = default;

    error_string(TCHAR const* message)
        : std::unique_ptr<element_type, local_free_wrapped>(message)
    {
    }

    error_string(HRESULT result)
        : error_string(format_hresult(result))
    {
    }
};

class error_string_ex : public error_string
{
    HRESULT id_;

  public:
    error_string_ex(HRESULT id)
        : error_string(id)
        , id_(id)
    {
    }

    bool operator==(HRESULT other_id) const
    {
        return id_ == other_id;
    }
};

static array<error_string, 0xFFFF> error_msg_cache;
static vector<error_string_ex> error_msg_cache2;

static char const* format_message(HRESULT result)
{
    TCHAR const* message;
    if (DWORD win32_error; WIN32_FROM_HRESULT(result, &win32_error))
    {
        auto& cached = error_msg_cache[win32_error];
        if (cached)
            cached = result;
        message = cached.get();
    }
    else
    {
        auto end = error_msg_cache2.end();
        auto it  = std::find(error_msg_cache2.begin(), end, result);
        message  = it != end ? iterator_to_raw_pointer(it)->get() : error_msg_cache2.emplace_back(result).get();
    }

    return message;
}

system_error::system_error(HRESULT code, char const* message) noexcept
    : runtime_error(message)
    , code_(code)
{
}

system_error::system_error(DWORD code, char const* message) noexcept
    : runtime_error(message)
    , code_(HRESULT_FROM_WIN32(code))
{
}

system_error::system_error(char const* message) noexcept
    : runtime_error(message)
    , code_(HRESULT_FROM_WIN32(GetLastError()))
{
}

auto system_error::code() const noexcept -> code_type
{
    return code_;
}

char const* system_error::error() const noexcept
{
    return format_message(code_);
}
} // namespace fd