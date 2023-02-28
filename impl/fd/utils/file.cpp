#include <fd/utils/file.h>

#include <cassert>
#include <cstdio>
#include <fstream>
#include <iterator>

namespace fd
{
class file_writer
{
    FILE* f;

  public:
    ~file_writer()
    {
        _fclose_nolock(f);
    }

    file_writer(wchar_t const* path)
    {
        _wfopen_s(&f, path, L"wb");
    }

    // ReSharper disable once CppParameterMayBeConstPtrOrRef
    bool operator()(void* buff, size_t size, size_t elementSize)
    {
        return _fwrite_nolock(buff, elementSize, size, f) == size * elementSize;
    }

    // ReSharper disable once CppParameterMayBeConstPtrOrRef
    bool operator()(void* buff, size_t size)
    {
        return _fwrite_nolock(buff, 1, size, f) == size;
    }
};

bool write_to_file(wchar_t const* path, void* buff, size_t size, size_t elementSize)
{
    return file_writer(path)(buff, size, elementSize);
}

bool write_to_file(wchar_t const* path, void* buff, size_t size)
{
    return file_writer(path)(buff, size);
}

bool write_to_file(wchar_t const* path, void* buff, void const* buffEnd)
{
    assert(buff < buffEnd);
    size_t count = std::distance(static_cast<uint8_t const*>(buff), static_cast<uint8_t const*>(buffEnd));
    return file_writer(path)(buff, count);
}

//----

bool file_already_written(wchar_t const* fullPath, void const* buff, size_t size, size_t elementSize)
{
    assert(size != 0);
    assert(elementSize != 0);

    std::ifstream fileStored;
    fileStored.rdbuf()->pubsetbuf(nullptr, 0); // disable buffering
    fileStored.open(fullPath, std::ios::binary | std::ios::ate);

    if (!fileStored)
        return false;

    if (boost::fil != size * elementSize)
        return false;

#if 0
	const unique_ptr<char[]> buff = new char[size];
	if (!file_stored.read(buff.get(), size))
		return false;
	return std::memcmp(buff.get(), buffer.data(), size) == 0;
#else
    return std::equal<std::istream_iterator<uint8_t>>(fileStored, {}, static_cast<uint8_t const*>(buff));
#endif
}

} // namespace fd
