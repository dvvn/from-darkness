#include "cached_text.h"

#include "imgui_id.h"

#include <nstd/enum_tools.h>
#include <nstd/memory backup.h>
#include <nstd/runtime assert.h>
#include <nstd/overload.h>

#include <imgui_internal.h>

#include <numeric>
#include <ranges>
#include <algorithm>
#include <array>
#include <cmath>

using namespace cheat::gui::tools;

#if 0
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

#endif

void cached_text::set_font(ImFont* new_font)
{
	font = new_font;
	add_update_flag(update_flags::FONT_CHANGED);
	if (label.empty( ))
		return;
	this->update( );
}

void cached_text::update()
{
	using namespace nstd::enum_operators;

	runtime_assert(!label.empty( ));
	runtime_assert(font != nullptr);
	runtime_assert(update_flags_ & (update_flags::CHANGED));

	using char_type = label_type::value_type;
	const auto get_glyphs_for = [&](const std::basic_string_view<char_type>& str)
	{
		const auto get_glyph_safe = [&](char_type chr)-> ImFontGlyph&
		{
			const auto glyph =
#if 0
				font->FindGlyphNoFallback(chr);
			runtime_assert(glyph != nullptr);
#else
					font->FindGlyph(chr);
#endif
			return *const_cast<ImFontGlyph*>(glyph);
		};

		return str | std::views::transform(get_glyph_safe);
	};

	//cache_glyphs
	auto glyphs = get_glyphs_for(label);
	//glyphs_.assign(glyphs.begin( ), glyphs.end( ));
	auto visible_glyphs     = /*glyphs_*/glyphs | std::views::filter([](const ImFontGlyph& gl)-> bool { return gl.Visible; });
	auto renderavle_glyphs  = /*glyphs_*/glyphs | std::views::filter([](const ImFontGlyph& gl)-> bool { return gl.AdvanceX > 0; });
	visible_glyphs_count    = std::ranges::distance(visible_glyphs);
	randerable_glyphs_count = std::ranges::distance(renderavle_glyphs);

	//update_label_size
	auto advances = glyphs | std::views::transform(&ImFontGlyph::AdvanceX); //AdvanceX is valid after atlas rebuild
	label_size.x  = std::accumulate(advances.begin( ), advances.end( ), 0.f);
	label_size.y  = font->FontSize; //line_height

	if (update_flags_ & update_flags::LABEL_CHANGED)
		label_hash = std::_Hash_array_representation(label._Unchecked_begin( ), label.size( ));

	this->on_update(update_flags_);

	update_flags_ = update_flags::NONE;
}

void cached_text::add_update_flag(update_flags flag)
{
	using namespace nstd::enum_operators;
	update_flags_ |= flag;
}

template <typename T>
using make_index_sequence_tuple = std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<T>>>;

template <typename Rng, typename Fn, size_t ...I>
static bool _Any_of_noloop(Rng&& rng, Fn&& pred, std::index_sequence<I...>)
{
	return (std::invoke(pred, rng[I]) || ...);
}

template <typename Rng, typename Fn>
static bool _Any_of_noloop(Rng&& rng, Fn&& pred)
{
	return _Any_of_noloop(rng, pred, make_index_sequence_tuple<Rng>( ));
}

template <typename Rng, typename Fn, size_t ...I>
static void _For_each_noloop(Rng&& rng, Fn&& fn, std::index_sequence<I...>)
{
	(std::invoke(fn, rng[I]), ...);
}

template <typename Rng, typename Fn>
static void _For_each_noloop(Rng&& rng, Fn&& fn)
{
	_For_each_noloop(rng, fn, make_index_sequence_tuple<Rng>( ));
}

size_t cached_text::render(ImDrawList* draw_list, ImVec2 pos, ImU32 color, const ImVec2& align, const ImRect& clip_rect_override, bool cram_clip_rect_x
						 , bool cram_clip_rect_y) const
{
	if ((color & IM_COL32_A_MASK) == 0)
		return 0;

	runtime_assert(font->ContainerAtlas->TexID == draw_list->_CmdHeader.TextureId);

	auto& clip_rect       = draw_list->_CmdHeader.ClipRect;
	auto clip_rect_backup = [&]
	{
		std::array<nstd::memory_backup<float>, 4> backup;

		auto& [min, max] = clip_rect_override;
		if (min.x != FLT_MAX && (cram_clip_rect_x || min.x > clip_rect.x))
			backup[0] = {clip_rect.x, min.x};
		if (min.y != FLT_MAX && (cram_clip_rect_y || min.y > clip_rect.y))
			backup[1] = {clip_rect.y, min.y};
		if (max.x != FLT_MAX && (cram_clip_rect_x || max.x < clip_rect.z))
			backup[2] = {clip_rect.z, max.x};
		if (max.y != FLT_MAX && (cram_clip_rect_y || max.y < clip_rect.w))
			backup[3] = {clip_rect.w, max.y};
		return backup;
	}( );

	const auto add_draw_cmd = _Any_of_noloop(clip_rect_backup, &nstd::memory_backup<float>::has_value);
	if (add_draw_cmd)
	{
		draw_list->_OnChangedClipRect( );
	}

	const auto align_helper = [&](float ImVec2::* dir, float ImVec4::* clip_rect_dir)
	{
		const auto align_val = std::invoke(dir, align);

		if (align_val == 0)
			return;

		const auto label_size_val = std::invoke(dir, label_size);
		const auto clip_val       = std::invoke(clip_rect_dir, clip_rect);

		auto& val = std::invoke(dir, pos);

		if (align_val == 1)
			val = clip_val - label_size_val;
		else /*if (align_val != 0)*/
			val += std::max(0.f, clip_val - /*clip_rect.XY*/val - label_size_val) * align_val;
	};

	align_helper(&ImVec2::x, &ImVec4::z);
	align_helper(&ImVec2::y, &ImVec4::w);

	// Align to be pixel perfect
	pos.x = std::floor(pos.x);
	pos.y = std::floor(pos.y);

	const auto finish = [&]
	{
		if (add_draw_cmd)
		{
			_For_each_noloop(clip_rect_backup, &nstd::memory_backup<float>::restore);
			draw_list->_OnChangedClipRect( );
		}
	};

	const auto label_rect = ImRect(pos, pos + label_size);
	if (label_rect.Max.y <= 0 || label_rect.Max.x <= 0 || label_rect.Min.x >= clip_rect.z || label_rect.Min.y >= clip_rect.w)
	{
		finish( );
		return 0;
	}

	// Reserve vertices for remaining worse case (over-reserving is useful and easily amortized)
	draw_list->PrimReserve(visible_glyphs_count * 6, visible_glyphs_count * 4);

	const auto col_untinted = color | ~IM_COL32_A_MASK;

	size_t glyphs_rendered = 0;
	for (const auto glyph : /*glyphs_*/this->label | std::views::transform([&](label_type::value_type chr) { return font->FindGlyph(chr); }))
	{
		// We don't do a second finer clipping test on the Y axis as we've already skipped anything before clip_rect.y and exit once we pass clip_rect.w
		const auto x1 = pos.x + glyph->X0;
		const auto x2 = pos.x + glyph->X1;

		if (glyph->Visible && x1 <= clip_rect.z && x2 >= clip_rect.x)
		{
			const auto y1 = pos.y + glyph->Y0;
			const auto y2 = pos.y + glyph->Y1;

			const auto u1 = glyph->U0;
			const auto v1 = glyph->V0;
			const auto u2 = glyph->U1;
			const auto v2 = glyph->V1;

			// Support for untinted glyphs
			const auto glyph_col = glyph->Colored ? col_untinted : color;
			draw_list->PrimRectUV({x1, y1}, {x2, y2}, {u1, v1}, {u2, v2}, glyph_col);

			++glyphs_rendered;
		}

		pos.x += glyph->AdvanceX;
	}

	if (glyphs_rendered < visible_glyphs_count)
	{
		const auto unused_glyphs = visible_glyphs_count - glyphs_rendered;
		const auto idx_dummy     = unused_glyphs * 6;
		const auto vtx_dummy     = unused_glyphs * 4;
		draw_list->PrimUnreserve(idx_dummy, vtx_dummy);
	}

	finish( );
#if 0
	//todo: left right / up down
		ImGui::SetNextWindowPos(label_rect.Max, ImGuiCond_Always);
	

	if (ImGui::IsMouseHoveringRect(label_rect.Min, label_rect.Max, false))
	{
		ImGui::BeginTooltip( );
		ImGui::Text("Visible %d glyphs", glyphs_rendered);
		ImGui::EndTooltip( );
	}
#endif

	return glyphs_rendered;
}
