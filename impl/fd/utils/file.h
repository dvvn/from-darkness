#pragma once

namespace fd
{
bool write_to_file(wchar_t const* path, void const* buff, size_t size, size_t elementSize = 1);
bool write_to_file(wchar_t const* path, void* buff, void const* buffEnd);

bool file_already_written(wchar_t const* fullPath, void const* buff, size_t size, size_t elementSize = 1);
} // namespace fd