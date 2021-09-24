#include "cached_text.h"

#include <nstd/overload.h>
#include <nstd/runtime assert.h>

#include <ww898/utf_converters.hpp>

#include <robin_hood.h>

#include <Windows.h>

#include <functional>
#include <algorithm>
#include <imgui_internal.h>
#include <ranges>
#include <numeric>

using namespace cheat::gui::tools;

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

string_wrapper::operator value_type() const
{
	return this->imgui( );
}

string_wrapper::value_type string_wrapper::imgui() const
{
	return _Get_imgui_str(multibyte_);
}

//-------------

perfect_string::perfect_string(const string_wrapper& wrstr)
	: holder_(std::ref(wrstr))
{
	chars_count_    = wrstr.raw( ).size( );
	chars_capacity_ = wrstr.multibyte( ).size( );
}

perfect_string::perfect_string(string_wrapper&& wrstr)
	: holder_(std::move(wrstr))
{
	const auto wrp = std::get<string_wrapper>(holder_);

	chars_count_    = wrp.raw( ).size( );
	chars_capacity_ = wrp.multibyte( ).size( );
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
	chars_count_    = _UTF8_decode(strv).size( );
	chars_capacity_ = strv.size( );
}

perfect_string::operator string_wrapper::value_type() const
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

size_t perfect_string::chars_count() const
{
	return chars_count_;
}

size_t perfect_string::chars_capacity() const
{
	return chars_capacity_;
}

//--------------

void cached_text::set_font(ImFont* new_font)
{
	font_ = new_font;
	this->update( );
}

namespace ww898::utf::detail
{
	template < >
	struct utf_selector<char8_t> final
	{
		using type = utf8;
	};
}

template <typename Chr, typename Chr2>
static void _Convert_text(const std::basic_string_view<Chr>& in, std::basic_string<Chr2>& out)
{
	if constexpr (sizeof(Chr) == sizeof(Chr2))
	{
		auto transformed = in | std::views::transform([](Chr c) { return static_cast<Chr2>(c); });
		out.assign(transformed.begin( ), transformed.end( ));
	}
	else
		out = ww898::utf::conv<Chr2>(in);
}

template <typename Chr, typename Chr2>
static void _Convert_text(std::basic_string<Chr>&& in, std::basic_string<Chr2>& out)
{
	if constexpr (std::same_as<Chr, Chr2>)
		std::swap(in, out);
	else
		_Convert_text(std::basic_string_view<Chr>(in), out);
}

template <class T>
static void _Detect_zero_number(const T& in)
{
	for (auto c : in)
	{
		if (c == T::value_type('\0'))
			throw std::logic_error("'0' inside string detected");
	}
}

#define CONVERT_AND_UPDATE\
	_Detect_zero_number(str);\
	_Convert_text(std::forward<decltype(str)>(str), label_);\
	this->update()

void cached_text::set_label(const std::string_view& str)
{
	CONVERT_AND_UPDATE;
}

void cached_text::set_label(const std::u8string_view& str)
{
	CONVERT_AND_UPDATE;
}

void cached_text::set_label(const std::wstring_view& str)
{
	CONVERT_AND_UPDATE;
}

void cached_text::set_label(const std::u16string_view& str)
{
	CONVERT_AND_UPDATE;
}

void cached_text::set_label(const std::u32string_view& str)
{
	CONVERT_AND_UPDATE;
}

void cached_text::set_label(std::string&& str)
{
	CONVERT_AND_UPDATE;
}

void cached_text::set_label(std::u8string&& str)
{
	CONVERT_AND_UPDATE;
}

void cached_text::set_label(std::wstring&& str)
{
	CONVERT_AND_UPDATE;
}

void cached_text::set_label(std::u16string&& str)
{
	CONVERT_AND_UPDATE;
}

void cached_text::set_label(std::u32string&& str)
{
	CONVERT_AND_UPDATE;
}

void cached_text::update()
{
	if (!font_ || label_.empty( ))
		return;

	const auto get_glyphs_for = [&](const std::basic_string_view<char_type>& str)
	{
		const auto get_glyph_safe = [&](char_type chr)-> ImFontGlyph&
		{
#if 0
			 auto glyph = font_->FindGlyphNoFallback(chr);
			runtime_assert(glyph != nullptr);
#else
			auto glyph = font_->FindGlyph(chr);
#endif
			return *const_cast<ImFontGlyph*>(glyph);
		};

		return str | std::views::transform(get_glyph_safe);
	};

	const auto cache_glyphs = [&]
	{
		auto glyphs = get_glyphs_for(label_);
		//glyphs_.assign(glyphs.begin( ), glyphs.end( ));
		auto visible_glyphs   = /*glyphs_*/glyphs | std::views::filter([](const ImFontGlyph& gl)-> bool { return gl.Visible; });
		visible_glyphs_count_ = std::ranges::distance(visible_glyphs);
	};

	const auto update_label_size = [&]
	{
		auto advances = /*glyphs_*/get_glyphs_for(label_) | std::views::transform(&ImFontGlyph::AdvanceX); //AdvanceX is valid after atlas rebuild
		label_size_.x = std::accumulate(advances.begin( ), advances.end( ), 0.f);
		label_size_.y = font_->FontSize; //line_height
	};

	cache_glyphs( );
	update_label_size( );
}

void cached_text::render(ImDrawList* draw_list, ImVec2 pos, ImU32 color) const
{
	if ((color & IM_COL32_A_MASK) == 0)
		return;

	runtime_assert(font_->ContainerAtlas->TexID == draw_list->_CmdHeader.TextureId);

	const ImVec4 clip_rect = draw_list->_CmdHeader.ClipRect;

	// Align to be pixel perfect
	pos.x = IM_FLOOR(pos.x);
	pos.y = IM_FLOOR(pos.y);

	float x = pos.x;
	float y = pos.y;
	if (y > clip_rect.w)
		return;

	// Reserve vertices for remaining worse case (over-reserving is useful and easily amortized)
	draw_list->PrimReserve(visible_glyphs_count_ * 6, visible_glyphs_count_ * 4);

	const ImU32 col_untinted = color | ~IM_COL32_A_MASK;

	size_t glyphs_rendered = 0;

	for (const auto glyph : /*glyphs_*/this->label_ | std::views::transform([&](char_type chr) { return font_->FindGlyph(chr); }))
	{
		// We don't do a second finer clipping test on the Y axis as we've already skipped anything before clip_rect.y and exit once we pass clip_rect.w
		const auto x1 = x + glyph->X0;
		const auto x2 = x + glyph->X1;

		if (glyph->Visible && x1 <= clip_rect.z && x2 >= clip_rect.x)
		{
			const auto y1 = y + glyph->Y0;
			const auto y2 = y + glyph->Y1;

			const auto u1 = glyph->U0;
			const auto v1 = glyph->V0;
			const auto u2 = glyph->U1;
			const auto v2 = glyph->V1;

			// Support for untinted glyphs
			const auto glyph_col = glyph->Colored ? col_untinted : color;
			draw_list->PrimRectUV({x1, y1}, {x2, y2}, {u1, v1}, {u2, v2}, glyph_col);

			++glyphs_rendered;
		}

		x += glyph->AdvanceX;
	}

	if (glyphs_rendered < visible_glyphs_count_)
	{
		const auto unused_glyphs = visible_glyphs_count_ - glyphs_rendered;
		const auto idx_dummy     = unused_glyphs * 6;
		const auto vtx_dummy     = unused_glyphs * 4;
		draw_list->PrimUnreserve(idx_dummy, vtx_dummy);
	}
}
