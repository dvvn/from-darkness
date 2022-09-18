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
    class reader
    {
        file_stream stream_;

      public:
        reader();
        reader& operator=(file_stream&& stream);
    };

    class writer
    {
        file_stream stream_;

      public:
        writer();
        writer(file_stream&& stream);
        writer& operator=(file_stream&& stream);

        void write(const wchar_t* ptr, const size_t size);
        void write(const char* ptr, const size_t size);
    };

    class logs_writer : writer
    {
        mutex mtx_;

        using writer::write;

        template <class T, class S>
        void write_impl(const T& time, const S text)
        {
            write(time.data(), time.size());
            write(" - ", 3);
            write(text.data(), text.size());
            write("\n", 1);
        }

      public:
        using writer::writer;
        using writer::operator=;

        void write_unsafe(const string_view text);
        void write_unsafe(const wstring_view text);

        void write(const string_view text);
        void write(const wstring_view text);
    };

    export class system_console
    {
        reader in_;
        logs_writer out_;
        logs_writer err_;

        HWND window_ = nullptr;

      public:
        ~system_console();
        system_console();

        void write(const string_view str);
        void write(const wstring_view wstr);
    };
} // namespace fd
