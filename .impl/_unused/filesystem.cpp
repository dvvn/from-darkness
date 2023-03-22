#include <fd/assert.h>
#include <fd/filesystem.h>

#include <Windows.h>
#include <winternl.h>

#include <ranges>

#pragma comment(lib, "Ntdll.lib")

namespace fd
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

    const wchar_t* data() const
    {
        return buff_.data();
    }

    size_t size() const
    {
        return buff_.size();
    }

    template <class T>
    void append(const T begin, size_t size)
    {
        buff_.append(begin, size);
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

    wstring_view name() const
    {
        return { nameBuffer_.data(), nameBuffer_.size() };
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

    bool operator!() const
    {
        return !h_ || h_ == INVALID_HANDLE_VALUE;
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
    auto        options     = _file_open_flags(isFile) | FILE_RANDOM_ACCESS;
    if (override)
        options |= FILE_SHARE_DELETE;
    const ULONG fileAttr = isFile ? FILE_ATTRIBUTE_NORMAL : FILE_ATTRIBUTE_DIRECTORY;
    const auto  status   = NtCreateFile(&h, desiredAccess, &attr, &statusBlock, nullptr, fileAttr, 0, disposition, options, nullptr, 0);
    if (NT_SUCCESS(status))
        return true;

    if (isFile)
        return false;

    wstring buff;
    auto    fixedPath = attr.name();

    if (!fixedPath.ends_with('\\'))
    {
        buff.reserve(fixedPath.size() + 1);
        buff.append(fixedPath);
        buff.push_back('\\');
        fixedPath = buff;
    }

    auto       validate = true;
    const auto firstIt  = _begin(fixedPath);
    for (auto it = firstIt + 4 /*skip '\??\'*/; it < _end(fixedPath); ++it)
    {
        if (*it != '\\')
            continue;
        const wstring_view prevPath(firstIt, it);
        ++it; // skip '\'
        if (validate && _path_exists(prevPath, false))
            continue;
        nt_handle             h2;
        win_object_attributes attr2(prevPath, OBJ_CASE_INSENSITIVE);
        const auto            status2 = NtCreateFile(&h2, desiredAccess, &attr2, &statusBlock, nullptr, fileAttr, 0, disposition, options, nullptr, 0);
        if (!NT_SUCCESS(status2))
            return false;
        validate = false;
    }
    return true;
}

template <class P>
static uintmax_t _file_size(const P path)
{
    nt_handle             h;
    win_object_attributes attr(path, OBJ_CASE_INSENSITIVE);
    IO_STATUS_BLOCK       statusBlock;
    constexpr ACCESS_MASK desiredAccess = FILE_READ_ATTRIBUTES | SYNCHRONIZE;
    const auto            options       = _file_open_flags(true);
    const auto            status        = NtOpenFile(&h, desiredAccess, &attr, &statusBlock, FILE_SHARE_READ, options);
    if (!NT_SUCCESS(status))
        return std::numeric_limits<uintmax_t>::max();

    static_assert(sizeof(LARGE_INTEGER) == sizeof(uintmax_t));

    union
    {
        LARGE_INTEGER winNsize;
        uintmax_t     size;
    };

    if (!GetFileSizeEx(h, &winNsize))
        return std::numeric_limits<uintmax_t>::max();

    return size;
}

template <class P>
static bool _directory_empty(const P path)
{
#if 0
    nt_handle             h;
    win_object_attributes attr(path, OBJ_CASE_INSENSITIVE);
    IO_STATUS_BLOCK       statusBlock;
    constexpr ACCESS_MASK desiredAccess = FILE_READ_ATTRIBUTES | SYNCHRONIZE;
    const auto            options       = _file_open_flags(true);
    const auto            status        = NtOpenFile(&h, desiredAccess, &attr, &statusBlock, FILE_SHARE_READ, options);
    if (!NT_SUCCESS(status))
        return 1;

    IO_STATUS_BLOCK        statusBlock2;
    FILE_INFORMATION_CLASS fileInfo;

    const auto status2 = NtQueryDirectoryFile(h, NULL, NULL, NULL, &statusBlock2, &fileInfo, sizeof(fileInfo), FileDirectoryInformation, TRUE, NULL, TRUE);
    if (!NT_SUCCESS(status2))
        return 1;

    switch(fileInfo) {
    case 1:

    }
#endif

    // WIP

    win_string str(path);
    if ((*reverse_iterator(_end(str))) == '\\')
        str.append(L"*.*", 3);
    else
        str.append(L"\\*.*", 4);

    WIN32_FIND_DATAW data;
    const nt_handle  handle(FindFirstFileW(str.data(), &data));
    return !handle || FindNextFileW(handle, &data) == ERROR_FILE_NOT_FOUND;
}

bool _file_exists(const char* begin, const char* end)
{
    return _path_exists(string_view(begin, end), true);
}

bool _file_exists(const wchar_t* begin, const wchar_t* end)
{
    return _path_exists(wstring_view(begin, end), true);
}

bool _file_empty(const char* begin, const char* end)
{
    return _file_size(string_view(begin, end)) == 0;
}

bool _file_empty(const wchar_t* begin, const wchar_t* end)
{
    return _file_size(wstring_view(begin, end)) == 0;
}

bool _directory_create(const char* begin, const char* end)
{
    return _path_create(string_view(begin, end), false, false);
}

bool _directory_create(const wchar_t* begin, const wchar_t* end)
{
    return _path_create(wstring_view(begin, end), false, false);
}

bool _directory_empty(const char* begin, const char* end)
{
    return _directory_empty(string_view(begin, end));
}

bool _directory_empty(const wchar_t* begin, const wchar_t* end)
{
    return _directory_empty(wstring_view(begin, end));
}
}