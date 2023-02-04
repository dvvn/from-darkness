#include <fd/assert.h>
#include <fd/filesystem.h>

#include <Windows.h>
#include <winternl.h>

#include <ranges>

#pragma comment(lib, "Ntdll.lib")

namespace fd::fs
{
class win_string
{
    wstring buff_;

  public:
    win_string() = default;

    template <class T>
    win_string(const T str)
    {
        assign(str);
    }

    template <class T>
    void assign(const T str, const bool isNative = false)
    {
        if (!buff_.empty())
            buff_.clear();

        if (str.starts_with('\\'))
        {
            buff_.reserve(str.size());
        }
        else
        {
            buff_.reserve(4 + str.size());
            buff_.append(L"\\??\\", 4);
        }

        if (isNative)
        {
            buff_.append(str.begin(), str.end());
        }
        else
        {
            for (auto ch : str)
                buff_ += ch == '/' ? '\\' : ch;
        }
    }

    bool empty() const
    {
        return buff_.empty();
    }

    wchar_t* data()
    {
        return buff_.data();
    }

    size_t size() const
    {
        return buff_.size();
    }
};

static void swap_length(UNICODE_STRING& l, UNICODE_STRING& r) noexcept
{
    std::swap(l.Length, r.Length);
    std::swap(l.MaximumLength, r.MaximumLength);
}

class win_object_attributes
{
    win_string        nameBuffer_;
    UNICODE_STRING    name_;
    OBJECT_ATTRIBUTES attr_;

    void init_base(const ULONG attributes)
    {
        attr_.Length                   = sizeof(OBJECT_ATTRIBUTES);
        attr_.RootDirectory            = nullptr;
        attr_.ObjectName               = &name_;
        attr_.Attributes               = attributes;
        attr_.SecurityDescriptor       = nullptr;
        attr_.SecurityQualityOfService = nullptr;
        static_assert(offsetof(OBJECT_ATTRIBUTES, SecurityQualityOfService) + sizeof(void*) == sizeof(OBJECT_ATTRIBUTES));
    }

    void fix_name()
    {
        if (!nameBuffer_.empty())
            name_.Buffer = nameBuffer_.data();
        attr_.ObjectName = &name_;
    }

    void kill_name()
    {
        name_.Buffer     = nullptr;
        attr_.ObjectName = nullptr;
    }

    template <class S>
    void init_name(const S str)
    {
        auto isNative = false;

        if constexpr (std::same_as<typename S::value_type, wchar_t>)
        {
            isNative = !str.contains(L'/');
            if (str[0] == '\\' && isNative)
            {
                name_.Buffer        = const_cast<wchar_t*>(str.data());
                name_.MaximumLength = name_.Length = static_cast<USHORT>(str.size() * sizeof(wchar_t));
                return;
            }
        }

        nameBuffer_.assign(str, isNative);
        name_.Buffer        = nameBuffer_.data();
        name_.Length        = static_cast<USHORT>(nameBuffer_.size() * sizeof(wchar_t));
        name_.MaximumLength = name_.Length + sizeof(wchar_t);
    }

  public:
    win_object_attributes(const win_object_attributes& other)            = delete;
    win_object_attributes& operator=(const win_object_attributes& other) = delete;

    win_object_attributes(win_object_attributes&& other) noexcept
        : nameBuffer_(std::move(other.nameBuffer_))
        , name_(other.name_)
        , attr_(other.attr_)
    {
        fix_name();
        other.kill_name();
    }

    win_object_attributes& operator=(win_object_attributes&& other) noexcept
    {
        using std::swap;
        swap(attr_, other.attr_);
        swap(nameBuffer_, other.nameBuffer_);
        swap_length(name_, other.name_);
        fix_name();
        other.fix_name();
        return *this;
    }

    win_object_attributes(const string_view path, const ULONG attributes = 0) // NOLINT(hicpp-member-init)
    {
        init_name(path);
        init_base(attributes);
    }

    win_object_attributes(const wstring_view path, const ULONG attributes = 0) // NOLINT(hicpp-member-init)
    {
        init_name(path);
        init_base(attributes);
    }

    OBJECT_ATTRIBUTES* operator&()
    {
        return &attr_;
    }
};

class nt_handle
{
    HANDLE h_;

  public:
    nt_handle(HANDLE h = INVALID_HANDLE_VALUE)
        : h_(h)
    {
    }

    ~nt_handle()
    {
        if (h_ && h_ != INVALID_HANDLE_VALUE)
            NtClose(h_);
    }

    nt_handle(const nt_handle& other)            = delete;
    nt_handle& operator=(const nt_handle& other) = delete;

    nt_handle(nt_handle&& other) noexcept
        : h_(std::exchange(other.h_, INVALID_HANDLE_VALUE))
    {
    }

    nt_handle& operator=(nt_handle&& other) noexcept
    {
        using std::swap;
        swap(h_, other.h_);
        return *this;
    }

    operator HANDLE() const
    {
        return h_;
    }

    PHANDLE operator&()
    {
        return &h_;
    }
};

static ULONG _file_open_flags(const bool isFile)
{
    return (isFile ? FILE_NON_DIRECTORY_FILE : FILE_DIRECTORY_FILE) | FILE_SYNCHRONOUS_IO_NONALERT;
}

template <class P>
static bool _path_exists(const P path, const bool isFile)
{
    nt_handle             h;
    win_object_attributes attr(path, OBJ_CASE_INSENSITIVE);
    IO_STATUS_BLOCK       statusBlock;
    constexpr ACCESS_MASK desiredAccess = FILE_READ_ATTRIBUTES | SYNCHRONIZE;
    const auto            options       = _file_open_flags(isFile);
    const auto            status        = NtOpenFile(&h, desiredAccess, &attr, &statusBlock, FILE_SHARE_READ, options);
    return NT_SUCCESS(status);
}

static constexpr ACCESS_MASK _FileOnlyAccessFlags = FILE_READ_DATA | FILE_WRITE_DATA | FILE_APPEND_DATA | FILE_EXECUTE;

template <class P>
static bool _path_create(const P path, const bool isFile, const bool override)
{
    nt_handle             h;
    win_object_attributes attr(path, OBJ_CASE_INSENSITIVE);
    IO_STATUS_BLOCK       statusBlock;
    ACCESS_MASK           desiredAccess = FILE_GENERIC_WRITE;
    if (!isFile)
        desiredAccess &= ~_FileOnlyAccessFlags;
    const ULONG disposition = override ? FILE_SUPERSEDE : FILE_CREATE;
    const auto  options     = _file_open_flags(isFile) | FILE_RANDOM_ACCESS;
    const auto  status      = NtCreateFile(&h, desiredAccess, &attr, &statusBlock, nullptr, FILE_ATTRIBUTE_NORMAL, 0, disposition, options, nullptr, 0);
    return NT_SUCCESS(status);
}

bool directory_impl::operator()(const string_view dir) const
{
#if 1
    return _path_exists(dir, false);
#else
    const auto shortPath = _Is_short_path(dir);
#ifdef PathIsDirectory
    return shortPath ? PathIsDirectoryA(FIX_C_STR(dir)) : PathIsDirectoryW(FIX_LONG_PATH(dir));
#else
    const auto attr = shortPath ? GetFileAttributesA(FIX_C_STR(dir)) : GetFileAttributesW(FIX_LONG_PATH(dir));
    return _Is_directory(attr);
#endif
#endif
}

bool directory_impl::operator()(const wstring_view dir) const
{
    return _path_exists(dir, false);
}

bool directory_impl::create(const string_view dir, const bool override) const
{
    return _path_create(dir, false, override);
}

bool directory_impl::create(const wstring_view dir, const bool override) const
{
    return _path_create(dir, false, override);
}

bool directory_impl::empty(const string_view /*dir*/) const
{
    FD_ASSERT_PANIC("Not implemented");
}

bool directory_impl::empty(const wstring_view /*dir*/) const
{
    FD_ASSERT_PANIC("Not implemented");
}

//-----

bool file_impl::operator()(const string_view dir) const
{
    return _path_exists(dir, true);
}

bool file_impl::create(const wstring_view dir, const bool override) const
{
    return _path_create(dir, true, override);
}

bool file_impl::create(const string_view dir, const bool override) const
{
    return _path_create(dir, true, override);
}

bool file_impl::operator()(const wstring_view dir) const
{
    return _path_exists(dir, true);
}
}