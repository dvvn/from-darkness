﻿#include "cached_text.h"

#include <nstd/memory backup.h>
#include <nstd/runtime assert.h>
#include <nstd/overload.h>

#include <imgui_internal.h>

#include <numeric>
#include <ranges>
#include <algorithm>
#include <array>
using namespace cheat::gui::tools;

imgui_string::imgui_type imgui_string::imgui() const
{
	return get_imgui_string(multibyte_);
}

const imgui_string::multibyte_type& imgui_string::multibyte() const
{
	return multibyte_;
}

const imgui_string::native_type& imgui_string::raw() const
{
	return native_;
}

imgui_string_transparent::imgui_string_transparent(const imgui_string& str)
{
	this->set_chars_count(str.raw( ));
	this->set_chars_capacity(str.multibyte( ));

	buff_.emplace<const imgui_string*>(std::addressof(str));
}

imgui_string_transparent::imgui_string_transparent(imgui_string&& str)
{
	this->set_chars_count(str.raw( ));
	this->set_chars_capacity(str.multibyte( ));

	auto& mb = const_cast<imgui_string::multibyte_type&>(str.multibyte( ));
	buff_.emplace<imgui_string::multibyte_type>(std::move(mb));
}

size_t imgui_string_transparent::chars_count() const
{
	return chars_count_;
}

size_t imgui_string_transparent::chars_capacity() const
{
	return chars_capacity_;
}

imgui_string_transparent::operator imgui_string::imgui_type() const
{
	return std::visit(nstd::overload(
							  [](const imgui_string* str)
							  {
								  return str->imgui( );
							  }
							, [](const imgui_string::imgui_type& str)
							  {
								  return str;
							  }
							, [](const imgui_string::multibyte_type& str)
							  {
								  return get_imgui_string(str);
							  }), buff_);
}

//-------------------

void cached_text::set_font(ImFont* new_font)
{
	font_ = new_font;
	this->update( );
}

void cached_text::update()
{
	if (!font_ || label_.empty( ))
		return;

	using char_type = label_type::value_type;

	const auto get_glyphs_for = [&](const std::basic_string_view<char_type>& str)
	{
		const auto get_glyph_safe = [&](char_type chr)-> ImFontGlyph&
		{
			const auto glyph =
#if 0
				font_->FindGlyphNoFallback(chr);
			runtime_assert(glyph != nullptr);
#else
					font_->FindGlyph(chr);
#endif
			return *const_cast<ImFontGlyph*>(glyph);
		};

		return str | std::views::transform(get_glyph_safe);
	};

	const auto cache_glyphs = [&]
	{
		auto glyphs = get_glyphs_for(label_);
		//glyphs_.assign(glyphs.begin( ), glyphs.end( ));
		auto visible_glyphs      = /*glyphs_*/glyphs | std::views::filter([](const ImFontGlyph& gl)-> bool { return gl.Visible; });
		auto renderavle_glyphs   = /*glyphs_*/glyphs | std::views::filter([](const ImFontGlyph& gl)-> bool { return gl.AdvanceX > 0; });
		visible_glyphs_count_    = std::ranges::distance(visible_glyphs);
		randerable_glyphs_count_ = std::ranges::distance(renderavle_glyphs);
	};

	const auto update_label_size = [&]
	{
		auto advances = /*glyphs_*/get_glyphs_for(label_) | std::views::transform(&ImFontGlyph::AdvanceX); //AdvanceX is valid after atlas rebuild
		label_size_.x = std::accumulate(advances.begin( ), advances.end( ), 0.f);
		label_size_.y = font_->FontSize; //line_height
	};

	const auto update_label_hash = [&]
	{
		label_hash_ = std::_Hash_array_representation(label_._Unchecked_begin( ), label_.size( ));
	};

	cache_glyphs( );
	update_label_size( );
	update_label_hash( );
}

template <typename Rng, typename Pred, size_t ...I>
static bool _Any_of_noloop(Rng&& rng, Pred&& pred, std::index_sequence<I...>)
{
	return (std::invoke(pred, rng[I]) || ...);
}

void cached_text::render(ImDrawList* draw_list, ImVec2 pos, ImU32 color, const ImVec2& align, const ImVec2& pos_start, const ImVec2& pos_end) const
{
	if ((color & IM_COL32_A_MASK) == 0)
		return;

	runtime_assert(font_->ContainerAtlas->TexID == draw_list->_CmdHeader.TextureId);

	auto& cmd_clip_rect = draw_list->_CmdHeader.ClipRect;

	std::array<nstd::memory_backup<float>, 4> clip_rect_backup;
	auto make_backup = [index = 0, &clip_rect_backup](float& target, float replace) mutable
	{
		const auto i = index++;
		if (replace > 0)
			clip_rect_backup[i] = {target, replace};
	};
	make_backup(cmd_clip_rect.x, pos_start.x);
	make_backup(cmd_clip_rect.y, pos_start.y);
	make_backup(cmd_clip_rect.z, pos_end.x);
	make_backup(cmd_clip_rect.w, pos_end.y);

	const auto add_draw_cmd = _Any_of_noloop(clip_rect_backup, &nstd::memory_backup<float>::has_value, std::make_index_sequence<4>( ));
	if (add_draw_cmd)
		draw_list->_OnChangedClipRect( );

	auto& x = pos.x;
	auto& y = pos.y;

	if (align.x > 0.0f)
		x = ImMax(x, x + (cmd_clip_rect.z - x - label_size_.x) * align.x);
	if (align.y > 0.0f)
		y = ImMax(y, y + (cmd_clip_rect.w - y - label_size_.y) * align.y);
	// Align to be pixel perfect
	x = IM_FLOOR(x);
	y = IM_FLOOR(y);

	const auto finish = [&]
	{
		if (add_draw_cmd)
		{
			{
				decltype(clip_rect_backup) dummy;
				std::swap(clip_rect_backup, dummy);
			}
			draw_list->_OnChangedClipRect( );
		}
	};

	if (x > cmd_clip_rect.z || y > cmd_clip_rect.w)
	{
		return finish( );
	}

	// Reserve vertices for remaining worse case (over-reserving is useful and easily amortized)
	draw_list->PrimReserve(visible_glyphs_count_ * 6, visible_glyphs_count_ * 4);

	const auto col_untinted = color | ~IM_COL32_A_MASK;

	size_t glyphs_rendered = 0;
	for (const auto glyph : /*glyphs_*/this->label_ | std::views::transform([&](label_type::value_type chr) { return font_->FindGlyph(chr); }))
	{
		// We don't do a second finer clipping test on the Y axis as we've already skipped anything before clip_rect.y and exit once we pass clip_rect.w
		const auto x1 = x + glyph->X0;
		const auto x2 = x + glyph->X1;

		if (glyph->Visible && x1 <= cmd_clip_rect.z && x2 >= cmd_clip_rect.x)
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

	finish( );
}
