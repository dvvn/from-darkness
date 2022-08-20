module;

#include <type_traits>

#include <Windows.h>

module fd.system.last_error;

using namespace fd;

template <typename T>
static auto _Format_message(const DWORD id)
{
    basic_string<T> buff;

    constexpr DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
    size_t msg_size;
    T* msg;
    auto msg_ptr = reinterpret_cast<T*>(&msg);

    if constexpr (std::is_same_v<T, char>)
        msg_size = FormatMessageA(flags, NULL, id, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), msg_ptr, 0, nullptr);
    else if constexpr (std::is_same_v<T, wchar_t>)
        msg_size = FormatMessageW(flags, NULL, id, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), msg_ptr, 0, nullptr);
    else
        static_assert(std::_Always_false<T>, "Unsupported char type");
    buff.assign(msg, msg_size);
    LocalFree(msg);
    return buff;
}

last_error::operator size_type() const
{
    return id_;
}

last_error::operator string() const
{
    return _Format_message<char>(id_);
}

last_error::operator wstring() const
{
    return _Format_message<wchar_t>(id_);
}
