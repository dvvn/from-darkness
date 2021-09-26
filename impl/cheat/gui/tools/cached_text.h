#pragma once

#include <nstd/unistring.h>

#include <imgui.h>

#include <string>
#include <variant>

namespace cheat::gui::tools
{
	namespace detail
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
			multibyte_.uni_assign(std::forward<T>(str));
			native_.uni_assign(std::forward<T>(str));
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
		template <typename Chr>
		imgui_string_transparent(const std::basic_string_view<Chr>& str)
		{
			if constexpr (sizeof(Chr) == sizeof(char))
			{
				this->set_chars_count(str);
				this->set_chars_capacity(str);
				buff_.emplace<imgui_string::imgui_type>(detail::get_imgui_string(str));
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
		using label_type = nstd::unistring<ImWchar>;

		void set_font(ImFont* new_font);

		template <typename T>
		void set_label(T&& str)
		{
			label_.uni_assign(std::forward<T>(str));
			this->update( );
		}

	private:
		void update();

	public:
		void render(ImDrawList* draw_list, ImVec2 pos, ImU32 color) const;

	private:
		//todo: invisible chars ignored
		label_type label_;
		ImVec2 label_size_;

		//std::vector<ImFontGlyph> glyphs_;//ImFontGlyph invalid after every atlas build, so it uselles
		size_t visible_glyphs_count_ = 0;

		ImFont* font_ = 0;

	public:
		const ImVec2& get_label_size() const { return label_size_; }
		ImFont* get_font() const { return font_; }

		const label_type& get_label() const { return label_; }
	};
}
