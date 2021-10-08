#pragma once

#include <nstd/unistring.h>

#include <imgui_internal.h>

#include <string>
#include <variant>

namespace cheat::gui::tools
{
	template <std::ranges::range T>
		requires(sizeof(std::ranges::range_value_t<T>) == sizeof(char))
	auto get_imgui_string(const T& str)
	{
#ifdef IMGUI_HAS_IMSTR
		return ImStrv(str._Unchecked_begin( ), str._Unchecked_end( ));
#else
		return reinterpret_cast<const char*>(/*std::_Const_cast*/str._Unchecked_begin( ));
#endif
	}

	class imgui_string
	{
	public:
		imgui_string() = default;

		using multibyte_type = nstd::unistring<char>;
		using native_type = nstd::unistring<wchar_t>;
		using imgui_type =
#ifdef IMGUI_HAS_IMSTR
			ImStrv
#else
		const char*
#endif
		;

		template <typename T>
		imgui_string(T&& str)
		{
			//it safe to 'move' twice, only one string accept move
			multibyte_.assign(std::forward<T>(str));
			native_.assign(std::forward<T>(str));
		}

		imgui_type imgui() const;
		const multibyte_type& multibyte() const;
		const native_type& raw() const;

	private:
		multibyte_type multibyte_;
		native_type native_;
	};

	class imgui_string_transparent
	{
		template <typename Str, typename T>
		static void set_count_helper(const T& str, size_t& count)
		{
			// ReSharper disable CppRedundantParentheses
			if constexpr (sizeof(Str::value_type) == sizeof(T::value_type))
				count = str.size( );
			else
				count = Str(str).size( );
			// ReSharper restore CppRedundantParentheses
		}

		template <typename T>
		void set_chars_count(const T& str)
		{
			set_count_helper<imgui_string::native_type>(str, chars_count_);
		}

		template <typename T>
		void set_chars_capacity(const T& str)
		{
			set_count_helper<imgui_string::multibyte_type>(str, chars_capacity_);
		}

		template <typename T>
		void construct_multibyte(T&& str)
		{
			this->set_chars_count(str);
			this->set_chars_capacity(str);
			buff_.emplace<imgui_string::multibyte_type>(std::forward<T>(str));
		}

	public:
		imgui_string_transparent() = default;

		template <typename Chr>
		imgui_string_transparent(const std::basic_string_view<Chr>& str)
		{
			if constexpr (sizeof(Chr) == sizeof(char))
			{
				this->set_chars_count(str);
				this->set_chars_capacity(str);
				buff_.emplace<imgui_string::imgui_type>(get_imgui_string(str));
			}
			else
				this->construct_multibyte(str);
		}

		template <typename Chr>
		imgui_string_transparent(std::basic_string<Chr>&& str)
		{
			this->construct_multibyte(std::move(str));
		}

		imgui_string_transparent(const imgui_string& str);
		imgui_string_transparent(imgui_string&& str);

		template <typename Chr, size_t N>
		imgui_string_transparent(const Chr (&str)[N])
			: imgui_string_transparent(std::basic_string_view<Chr>(str, std::next(str, N - 1)))
		{
		}

	private:
		std::variant
		<
			const imgui_string*,
			imgui_string::imgui_type,
			imgui_string::multibyte_type
		> buff_;

	public:
		operator imgui_string::imgui_type() const;

	private:
		size_t chars_count_ = 0, chars_capacity_ = 0;

	public:
		size_t chars_count() const;
		size_t chars_capacity() const;
	};

	class cached_text
	{
	public:
		virtual ~cached_text() = default;
		cached_text()          = default;

		using label_type = nstd::unistring<ImWchar>;

		void set_font(ImFont* new_font);

		template <typename T>
		void set_label(T&& str)
		{
			label.assign(std::forward<T>(str));
			add_update_flag(update_flags::LABEL_CHANGED);
			if (!font)
				return;
			this->update( );
		}

		size_t render(ImDrawList* draw_list, ImVec2 pos, ImU32 color
					, const ImVec2& align = {}, const ImRect& clip_rect_override = {FLT_MAX,FLT_MAX,FLT_MAX,FLT_MAX}, bool cram_clip_rect_x = 0, bool cram_clip_rect_y = 0) const;

		//todo: invisible chars ignored

		label_type label;
		ImVec2 label_size;
		size_t label_hash = 0;

		//std::vector<ImFontGlyph> glyphs_;//ImFontGlyph invalid after every atlas build, so it uselles
		size_t visible_glyphs_count    = 0;
		size_t randerable_glyphs_count = 0;
		ImFont* font                   = nullptr;

	protected:
		enum class update_flags :uint8_t
		{
			NONE
		  , LABEL_CHANGED = 1 << 0
		  , FONT_CHANGED = 1 << 1
		  , CHANGED = LABEL_CHANGED | FONT_CHANGED
		};

		virtual void on_update(update_flags flags) { return; }

	private:
		void update();

		void add_update_flag(update_flags flag);

		update_flags update_flags_ = update_flags::NONE;
	};
}
