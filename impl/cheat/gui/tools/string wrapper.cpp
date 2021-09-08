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

string_wrapper::string_wrapper(raw_type&& str)
{
	raw_ = std::move(str);

	if (IsTextUnicode(raw_._Unchecked_begin( ), raw_.size( ) * sizeof(raw_type::value_type), nullptr))
		multibyte_ = _UTF8_encode(raw_);
	else
		multibyte_ = std::string(raw_.begin( ), raw_.end( ));

	Set_imgui_str_( );
}

string_wrapper::string_wrapper(multibyte_type&& str)
{
	auto temp = raw_type(str.begin( ), str.end( ));

	if (IsTextUnicode(str._Unchecked_begin( ), str.size( ), nullptr))
	{
		multibyte_ = _UTF8_encode(temp);
		raw_       = _UTF8_decode(multibyte_);
	}
	else
	{
		multibyte_ = std::move(str);
		raw_       = std::move(temp);
	}

	Set_imgui_str_( );
}

string_wrapper::string_wrapper(string_wrapper&& other) noexcept
{
	*this = std::move(other);
}

string_wrapper::string_wrapper(const string_wrapper& other) noexcept
{
	*this = other;
}

string_wrapper& string_wrapper::operator=(const string_wrapper& other) noexcept
{
	raw_       = other.raw_;
	multibyte_ = other.multibyte_;

	Set_imgui_str_( );
	return *this;
}

bool string_wrapper::operator==(const string_wrapper& other) const
{
	return multibyte_ == other.multibyte_;
}

bool string_wrapper::operator!=(const string_wrapper& other) const
{
	return !operator==(other);
}

std::weak_ordering string_wrapper::operator<=>(const string_wrapper& other) const
{
	return multibyte_ <=> other.multibyte_;
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
	return imgui_;
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
	return imgui_;
}

string_wrapper& string_wrapper::operator=(string_wrapper&& other) noexcept
{
	std::swap(raw_, other.raw_);
	std::swap(multibyte_, other.multibyte_);

	Set_imgui_str_( );
	return *this;
}

void string_wrapper::Set_imgui_str_( )
{
	imgui_ = _Get_imgui_str(multibyte_);
}

string_wrapper::value_type tools::_Get_imgui_str(const std::string_view& str)
{
#if !CHEAT_GUI_HAS_IMGUI_STRV
	runtime_assert(*str._Unchecked_end( ) == '\0');
	return const_cast<char*>(str._Unchecked_begin( ));
#else
	return string_wrapper::value_type(str._Unchecked_begin( ), str._Unchecked_end( ));
#endif
}

#if 0
string_wrapper_abstract::string_wrapper_abstract( )
	: name_(std::in_place_index<0>, string_wrapper( ))
{
}

string_wrapper_abstract::operator const string_wrapper&( ) const
{
	return get( );
}

const string_wrapper& string_wrapper_abstract::get( ) const
{
	using val_t = const string_wrapper;
	using ref_t = val_t&;
	return visit(nstd::overload([](ref_t ref)-> ref_t { return ref; },
								std::bind_front(&std::reference_wrapper<val_t>::get)), name_);
}

void string_wrapper_abstract::init(string_wrapper&& name)
{
	name_ = {std::move(name)};
}

void string_wrapper_abstract::init(const string_wrapper& name)
{
	name_ = {std::ref(name)};
}
#endif

//auto imgui::operator<=>(const string_wrapper_abstract& str, const string_wrapper_abstract& other) noexcept -> std::strong_ordering
//{
//	return str.get( ) <=> other.get( );
//}
//
//auto imgui::operator<=>(const string_wrapper_abstract& str, const string_wrapper& other) noexcept -> std::strong_ordering
//{
//	return str.get( ) <=> other;
//}
//
//auto imgui::operator<=>(const string_wrapper& other, const string_wrapper_abstract& str) noexcept -> std::strong_ordering
//{
//	return str <=> other;
//}

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

//void string_wrapper_abstract::init(const string_wrapper* name)
//{
//	this->init(*name);
//}

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
