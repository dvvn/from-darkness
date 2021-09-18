#include "string wrapper.h"

#include "nstd/overload.h"
#include "nstd/runtime assert.h"

#include <functional>

#include <Windows.h>

using namespace cheat;
using namespace gui;
using namespace tools;

// Convert a wide Unicode std::string to an UTF8 std::string
static std::string _UTF8_encode(const std::wstring_view& wstr)
{
	const auto size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.data( ), wstr.size( ), nullptr, 0, nullptr, nullptr);
	auto       str_to      = std::string(size_needed, '\0');
	WideCharToMultiByte(CP_UTF8, 0, wstr.data( ), wstr.size( ), str_to.data( ), size_needed, nullptr, nullptr);
	return str_to;
}

// Convert an UTF8 std::string to a wide Unicode String
static std::wstring _UTF8_decode(const std::string_view& str)
{
	const auto size_needed = MultiByteToWideChar(CP_UTF8, 0, str.data( ), str.size( ), nullptr, 0);
	auto       wstr_to     = std::wstring(size_needed, '\0');
	MultiByteToWideChar(CP_UTF8, 0, str.data( ), str.size( ), wstr_to.data( ), size_needed);
	return wstr_to;
}

static string_wrapper::value_type _Get_imgui_str(const std::string_view& str)
{
#ifdef IMGUI_HAS_IMSTR
	return string_wrapper::value_type(str._Unchecked_begin( ), str._Unchecked_end( ));
#else
	runtime_assert(*str._Unchecked_end( ) == '\0');
	return const_cast<char*>(str._Unchecked_begin( ));
#endif
}

string_wrapper::string_wrapper(std::wstring&& str)
{
	multibyte_ = _UTF8_encode(str);
	raw_       = std::move(str);
}

string_wrapper::string_wrapper(std::u8string&& str)
{
	auto tmp   = std::string(str.begin( ), str.end( ));
	raw_       = _UTF8_decode(tmp);
	multibyte_ = std::move(tmp);
}

string_wrapper::string_wrapper(std::string&& str)
{
	raw_       = _UTF8_decode(str);
	multibyte_ = std::move(str);
}

string_wrapper::operator value_type( ) const
{
	return this->imgui( );
}

string_wrapper::value_type string_wrapper::imgui( ) const
{
	return _Get_imgui_str(multibyte_);
}

//-------------

perfect_string::perfect_string(const string_wrapper& wrstr)
	: holder_(std::ref(wrstr))
{
	chars_count_ = wrstr.raw( ).size( );
}

perfect_string::perfect_string(string_wrapper&& wrstr)
	: holder_(std::move(wrstr))
{
	chars_count_ = std::get<string_wrapper>(holder_).raw( ).size( );
}

perfect_string::perfect_string(string_wrapper::const_type str)
	:
	// ReSharper disable once CppFunctionalStyleCast
	holder_(string_wrapper::value_type(str))
{
	std::string_view strv;
#ifndef IMGUI_HAS_IMSTR
	strv = str;
#else
	strv = {str.Begin,str.End};
#endif
	chars_count_ = _UTF8_decode(strv).size( );
}

perfect_string::operator string_wrapper::value_type( ) const
{
	return visit(nstd::overload(
		[](const string_wrapper_ref& val)
		{
			return val.get( ).imgui( );
		}, [](const string_wrapper& val)
		{
			return val.imgui( );
		},
		[](const string_wrapper::value_type& val)
		{
			return val;
		}
		), holder_);
}

size_t perfect_string::chars_count( ) const
{
	return chars_count_;
}
