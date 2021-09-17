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
	: string_wrapper( )
{
	raw_       = std::move(str);
	multibyte_ = _UTF8_encode(raw_);
}

string_wrapper::string_wrapper(std::u8string&& str)
	: string_wrapper( )
{
	multibyte_real_ = std::move(str);
	raw_            = _UTF8_decode(multibyte_);
}

string_wrapper::string_wrapper(std::string&& str)
	: string_wrapper( )
{
	multibyte_ = std::move(str);
	raw_       = _UTF8_decode(multibyte_);
}

string_wrapper::~string_wrapper( )
{
	multibyte_real_.~basic_string( );
}

string_wrapper::string_wrapper( )
// ReSharper disable once CppRedundantMemberInitializer
	: multibyte_real_( )
{
}

string_wrapper::string_wrapper(string_wrapper&& other) noexcept
	: string_wrapper( )
{
	raw_            = std::move(other.raw_);
	multibyte_real_ = std::move(other.multibyte_real_);
}

string_wrapper& string_wrapper::operator=(string_wrapper&& other) noexcept
{
	std::swap(raw_, other.raw_);
	std::swap(multibyte_real_, other.multibyte_real_);

	return *this;
}

string_wrapper::string_wrapper(const string_wrapper& other) noexcept
	: string_wrapper( )
{
	*this = other;
}

string_wrapper& string_wrapper::operator=(const string_wrapper& other) noexcept
{
	raw_            = other.raw_;
	multibyte_real_ = other.multibyte_real_;

	return *this;
}

bool string_wrapper::operator==(const string_wrapper& other) const
{
	return multibyte_real_ == other.multibyte_real_;
}

bool string_wrapper::operator!=(const string_wrapper& other) const
{
	return !operator==(other);
}

std::weak_ordering string_wrapper::operator<=>(const string_wrapper& other) const
{
	return multibyte_real_ <=> other.multibyte_real_;
}

string_wrapper::operator std::wstring_view( ) const
{
	return raw_;
}

string_wrapper::operator std::string_view( ) const
{
	return multibyte_;
}

string_wrapper::operator value_type( ) const
{
	return this->imgui( );
}

std::wstring_view string_wrapper::raw( ) const
{
	return raw_;
}

std::string_view string_wrapper::multibyte( ) const
{
	return multibyte_;
}

string_wrapper::value_type string_wrapper::imgui( ) const
{
	return _Get_imgui_str(multibyte_);
}

[[maybe_unused]]
static auto _Ref_or_direct(const string_wrapper::value_type& val)
{
	return std::ref(val);
}

[[maybe_unused]]
static auto _Ref_or_direct(string_wrapper::value_type&& val)
{
	return (val);
}

// ReSharper disable once CppFunctionalStyleCast
perfect_string::perfect_string(ref_or_direct_type str)
	: holder_(_Ref_or_direct(std::forward<decltype(str)>(str)))
{
}

perfect_string::perfect_string(const std::string_view& str)
	: holder_(_Get_imgui_str(str))
{
}

perfect_string::perfect_string(const std::wstring_view& str)
	: perfect_string(std::wstring(str))
{
#if !CHEAT_GUI_HAS_IMGUI_STRV
	size_ = str.size( );
#endif
}

perfect_string::perfect_string(std::wstring&& str)
	: holder_(string_wrapper(std::move(str)))
{
#if !CHEAT_GUI_HAS_IMGUI_STRV
	size_ = str.size( );
#endif
}

perfect_string::perfect_string(const string_wrapper& str)
	: holder_(std::ref(str))
{
#if !CHEAT_GUI_HAS_IMGUI_STRV
	size_ = str.raw( ).size( );
#endif
}

perfect_string::operator string_wrapper::value_type( ) const
{
	return visit(nstd::overload([](const string_wrapper_ref& val)
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

size_t perfect_string::size( ) const
{
#if !CHEAT_GUI_HAS_IMGUI_STRV
	if (!size_.has_value( ))
		return size_.emplace(string_wrapper(std::string(*this)).raw( ).size( ));
	return *size_;
#else
	const string_wrapper::value_type val = *this;
	return val.length( );
#endif
}

bool perfect_string::operator==(const string_wrapper& wrapped) const
{
	return visit(nstd::overload([&](const string_wrapper_ref& val)
		{
			return val.get( ) == wrapped;
		},
		[&](const string_wrapper& val)
		{
			return wrapped == val;
		},
		[&](const string_wrapper::value_type& val)
		{
#if CHEAT_GUI_HAS_IMGUI_STRV
									TODO
#else
			return wrapped.multibyte( ) == val;
#endif
		}
		), holder_);
}

bool perfect_string::operator!=(const string_wrapper& wrapped) const
{
	return !operator==(wrapped);
}

bool tools::operator==(const string_wrapper& a, const perfect_string& b)
{
	return b == a;
}

bool tools::operator!=(const string_wrapper& a, const perfect_string& b)
{
	return !(b == a);
}
