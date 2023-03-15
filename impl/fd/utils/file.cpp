#include <fd/utils/file.h>

#include <cassert>
#include <cstdio>

#include <fstream>
#include <iterator>

namespace fd
{
static bool null_terminated_string(auto str)
{
    return str.data()[str.size()] == '\0'; // NOLINT(clang-diagnostic-unsafe-buffer-usage)
}

template <bool AllowOverride, class T>
static bool write_file(T file, std::span<char const> buff)
{
    if constexpr (!std::is_lvalue_reference_v<T>)
        assert(null_terminated_string(file));
    assert(!buff.empty());
    if constexpr (!AllowOverride)
    {
        std::ifstream file_stored;
        file_stored.rdbuf()->pubsetbuf(nullptr, 0); // disable buffering
        file_stored.open(file.data(), std::ios::binary | std::ios::ate);

        using it = std::istream_iterator<uint8_t>;
        if (file_stored && //
            file_stored.tellg() == buff.size() &&
            std::equal<it>(file_stored, {}, reinterpret_cast<uint8_t const *>(buff.data())))
        {
            return false;
        }
    }

    FILE *f;
    if (_wfopen_s(&f, file.data(), L"wb") != 0)
        return false;
    auto written = _fwrite_nolock(buff.data(), 1, buff.size(), f);
    _fclose_nolock(f);
    return written == buff.size();
}

void write_file(std::wstring_view file, std::span<char> buff, bool allow_override)
{
    if (allow_override)
        write_file<true>(file, buff);
    else
        write_file<false>(file, buff);
}

void write_file(std::wstring_view file, std::span<char> buff)
{
    write_file<true>(file, buff);
}

#ifdef _DEBUG
void write_file(std::wstring const &file, std::span<char> buff, bool allow_override)
{
    if (allow_override)
        write_file<true, std::wstring const &>(file, buff);
    else
        write_file<false, std::wstring const &>(file, buff);
}

void write_file(std::wstring const &file, std::span<char> buff)
{
    write_file<true, std::wstring const &>(file, buff);
}
#endif

} // namespace fd