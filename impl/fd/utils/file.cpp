#include <fd/utils/file.h>

#include <cassert>
#include <cstdio>

#include <fstream>
#include <iterator>

namespace fd
{
static bool null_terminated_string(std::wstring_view str)
{
    return str.data()[str.size()] == '\0'; // NOLINT(clang-diagnostic-unsafe-buffer-usage)
}

static bool write_file(wchar_t const *path, void const *buff, size_t size, size_t elementSize)
{
    FILE *f;
    if (_wfopen_s(&f, path, L"wb"))
    {
        auto written = _fwrite_nolock(buff, elementSize, size, f);
        _fclose_nolock(f);
        return written == size * elementSize;
    }
    return false;
}

static bool file_already_written(wchar_t const *fullPath, void const *buff, size_t size, size_t elementSize)
{
    assert(size != 0);
    assert(elementSize != 0);

    std::ifstream fileStored;
    fileStored.rdbuf()->pubsetbuf(nullptr, 0); // disable buffering
    fileStored.open(fullPath, std::ios::binary | std::ios::ate);

    if (!fileStored)
        return false;

    if (fileStored.tellg() != size * elementSize)
        return false;

#if 0
	const unique_ptr<char[]> buff = new char[size];
	if (!file_stored.read(buff.get(), size))
		return false;
	return std::memcmp(buff.get(), buffer.data(), size) == 0;
#else
    return std::equal<std::istream_iterator<uint8_t>>(fileStored, {}, static_cast<uint8_t const *>(buff));
#endif
}

static void write_file_without_null_char_check(std::wstring_view file, std::span<char> buff, bool allow_override)
{
    if (!allow_override && file_already_written(file.data(), buff.data(), buff.size(), sizeof(char)))
        return;
    write_file(file.data(), buff.data(), buff.size(), sizeof(char));
}

void write_file(std::wstring_view file, std::span<char> buff, bool allow_override)
{
    assert(null_terminated_string(file));
    write_file_without_null_char_check(file, buff, allow_override);
}

void write_file(std::wstring_view file, std::span<char> buff)
{
    assert(null_terminated_string(file));
    write_file(file.data(), buff.data(), buff.size(), sizeof(char));
}

#ifdef _DEBUG
void write_file(std::wstring const &file, std::span<char> buff, bool allow_override)
{
    write_file_without_null_char_check(file, buff, allow_override);
}

void write_file(std::wstring const &file, std::span<char> buff)
{
    write_file(file.data(), buff.data(), buff.size(), sizeof(char));
}
#endif

} // namespace fd