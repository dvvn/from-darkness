#include "string wrapper.h"

using namespace cheat;
using namespace gui;
using namespace imgui;
using namespace utl;

// Convert a wide Unicode string to an UTF8 string
static string _UTF8_encode(const wstring_view& wstr)
{
	const auto size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.data( ), wstr.size( ), nullptr, 0, nullptr, nullptr);
	auto       str_to = string(size_needed, '\0');
	WideCharToMultiByte(CP_UTF8, 0, wstr.data( ), wstr.size( ), str_to.data( ), size_needed, nullptr, nullptr);
	return str_to;
}

// Convert an UTF8 string to a wide Unicode String
static wstring _UTF8_decode(const string_view& str)
{
	const auto size_needed = MultiByteToWideChar(CP_UTF8, 0, str.data( ), str.size( ), nullptr, 0);
	auto       wstr_to = wstring(size_needed, '\0');
	MultiByteToWideChar(CP_UTF8, 0, str.data( ), str.size( ), wstr_to.data( ), size_needed);
	return wstr_to;
}

string_wrapper::string_wrapper(raw_type&& str)
{
	raw__ = move(str);

	if (IsTextUnicode(raw__._Unchecked_begin( ), raw__.size( ) * sizeof(raw_type::value_type), nullptr))
		multibyte__ = _UTF8_encode(raw__);
	else
		multibyte__ = string(raw__.begin( ), raw__.end( ));

	Set_imgui_str_( );
}

string_wrapper::string_wrapper(multibyte_type&& str)
{
	auto temp = raw_type(str.begin( ), str.end( ));

	if (IsTextUnicode(str._Unchecked_begin( ), str.size( ), nullptr))
	{
		multibyte__ = _UTF8_encode(temp);
		raw__ = _UTF8_decode(multibyte__);
	}
	else
	{
		multibyte__ = move(str);
		raw__ = move(temp);
	}

	Set_imgui_str_( );
}

string_wrapper::string_wrapper(string_wrapper&& other) noexcept
{
	*this = move(other);
}

string_wrapper::string_wrapper(const string_wrapper& other) noexcept
{
	*this = other;
}

string_wrapper& string_wrapper::operator=(const string_wrapper& other) noexcept
{
	raw__ = other.raw__;
	multibyte__ = other.multibyte__;

	Set_imgui_str_( );
	return *this;
}

bool string_wrapper::operator==(const string_wrapper& other) const
{
	return raw( ) == other.raw( );
}

bool string_wrapper::operator!=(const string_wrapper& other) const
{
	return raw( ) != other.raw( );
}

string_wrapper::operator wstring_view( ) const
{
	return raw__;
}

string_wrapper::operator string_view( ) const
{
	return multibyte__;
}

string_wrapper::operator value_type( ) const
{
	return imgui__;
}

wstring_view string_wrapper::raw( ) const
{
	return raw__;
}

string_view string_wrapper::multibyte( ) const
{
	return multibyte__;
}

string_wrapper::value_type string_wrapper::imgui( ) const
{
	return imgui__;
}

string_wrapper& string_wrapper::operator=(string_wrapper&& other) noexcept
{
	raw__ = move(other.raw__);
	multibyte__ = move(other.multibyte__);

	Set_imgui_str_( );
	return *this;
}

void string_wrapper::Set_imgui_str_( )
{
#if defined(IMGUI_HAS_IMSTR) && IMGUI_HAS_IMSTR
	imgui__={multibyte__._Unchecked_begin(),multibyte__._Unchecked_end()};
#else
	imgui__ = multibyte__._Unchecked_begin( );
#endif
}

string_wrapper_abstract::string_wrapper_abstract( ): name__(std::in_place_index<0>, string_wrapper( ))
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
	return visit(overload([](ref_t ref)-> ref_t { return ref; },
						  bind_front(&reference_wrapper<val_t>::get)), name__);
}

void string_wrapper_abstract::init(string_wrapper&& name)
{
	name__.emplace<0>(move(name));
}

void string_wrapper_abstract::init(const string_wrapper& name)
{
	name__.emplace<1>(name);
}

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
