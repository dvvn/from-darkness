#pragma once

#include <span>
#include <string>

namespace fd
{
void write_file(std::wstring_view file, std::span<char> buff, bool allow_override);
void write_file(std::wstring_view file, std::span<char> buff);
#ifdef _DEBUG
void write_file(std::wstring const &file, std::span<char> buff, bool allow_override);
void write_file(std::wstring const &file, std::span<char> buff);
#endif
} // namespace fd