#pragma once

#include <fd/string.h>

#include <Windows.h>

#include <mutex>

namespace fd
{
class file_stream
{
    bool  redirected_;
    FILE* stream_;

    file_stream(const file_stream&)            = default;
    file_stream& operator=(const file_stream&) = default;

  public:
    ~file_stream();
    file_stream();
    file_stream(FILE* stream);
    file_stream(const char* file_name, const char* mode, FILE* oldStream);
    file_stream(file_stream&& other) noexcept;
    file_stream& operator=(file_stream&& other) noexcept;
                 operator void*() const;
                 operator FILE*() const;
};

// gap. unused
class console_reader
{
    file_stream stream_;

  public:
    console_reader();
    void set(file_stream&& stream);
};

class console_writer
{
    file_stream stream_;
    std::mutex  mtx_;

  public:
    console_writer();
    void set(file_stream&& stream);

    void write_nolock(const wchar_t* ptr, size_t size);
    void write_nolock(const char* ptr, size_t size);

    void write(const wchar_t* ptr, size_t size);
    void write(const char* ptr, size_t size);

    void lock();
    void unlock();
};

class system_console
{
    console_reader in_;
    console_writer out_;
    console_writer err_;

    HWND window_ = nullptr;

  public:
    ~system_console();
    system_console();

    void write_nolock(string_view str);
    void write_nolock(wstring_view wstr);

    void write(string_view str);
    void write(wstring_view wstr);
};

} // namespace fd