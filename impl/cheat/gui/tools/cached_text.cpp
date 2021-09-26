#include "cached_text.h"

#include <nstd/runtime assert.h>

#include <imgui_internal.h>

#include <numeric>
#include <ranges>

using namespace cheat::gui::tools;

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

	auto x = pos.x;
	auto y = pos.y;
	if (y > clip_rect.w)
		return;

	// Reserve vertices for remaining worse case (over-reserving is useful and easily amortized)
	draw_list->PrimReserve(visible_glyphs_count_ * 6, visible_glyphs_count_ * 4);

	const ImU32 col_untinted = color | ~IM_COL32_A_MASK;

	size_t glyphs_rendered = 0;
	for (const auto glyph : /*glyphs_*/this->label_ | std::views::transform([&](label_type::value_type chr) { return font_->FindGlyph(chr); }))
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
