module;

#include <Windows.h>

#include <cstdint>
#include <cstdio>

export module fd.system.console;
export import fd.string;
import fd.mutex;

namespace fd
{
    class file_stream
    {
        FILE* stream_;
        bool redirected_;

        file_stream(const file_stream&)            = default;
        file_stream& operator=(const file_stream&) = default;

      public:
        ~file_stream();
        file_stream();
        file_stream(FILE* stream);
        file_stream(const char* file_name, const char* mode, FILE* old_stream);
        file_stream(file_stream&& other);
        file_stream& operator=(file_stream&& other);
        operator void*() const;
        operator FILE*() const;
    };

    // gap. unused
    class file_stream_reader
    {
        file_stream stream_;

      public:
        file_stream_reader();
        file_stream_reader& operator=(file_stream&& stream);
    };

    class file_stream_writer : public basic_mutex
    {
        file_stream stream_;
        mutex mtx_;

      public:
        file_stream_writer();
        file_stream_writer(file_stream&& stream);
        file_stream_writer& operator=(file_stream&& stream);

        void lock() noexcept final;
        void unlock() noexcept final;

        void write_nolock(const wchar_t* ptr, const size_t size);
        void write_nolock(const char* ptr, const size_t size);

        void write(const wchar_t* ptr, const size_t size);
        void write(const char* ptr, const size_t size);
    };

    export class system_console
    {
        file_stream_reader in_;
        file_stream_writer out_;
        file_stream_writer err_;

        HWND window_ = nullptr;

      public:
        ~system_console();
        system_console();

        void write_nolock(const string_view str);
        void write_nolock(const wstring_view wstr);

        void write(const string_view str);
        void write(const wstring_view wstr);
    };

} // namespace fd
