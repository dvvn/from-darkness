#pragma once

#include <imgui.h>

#include <string>
#include <variant>
#include <vector>

namespace cheat::gui::tools
{
	namespace detail
	{
		class string_wrapper_multibyte
		{
		public:
			auto operator<=>(const string_wrapper_multibyte&) const = default;

			operator std::string_view() const { return multibyte_; }
			std::string_view multibyte() const { return multibyte_; }

		protected:
			// ReSharper disable once CppInconsistentNaming
			std::string multibyte_;
		};

		class string_wrapper_raw
		{
		public:
			operator std::wstring_view() const { return raw_; }
			std::wstring_view raw() const { return raw_; }

		protected:
			// ReSharper disable once CppInconsistentNaming
			std::wstring raw_;
		};
	}

	//deprecated!!!
	class string_wrapper : public detail::string_wrapper_multibyte, public detail::string_wrapper_raw
	{
	public:
#ifdef IMGUI_HAS_IMSTR
		using value_type = ImStrv;
		using const_type= const ImStrv&
#else
		using value_type = /*const*/ char*;
		using const_type = const char*;
#endif
		string_wrapper() = default;

		string_wrapper(std::string&& str);
		string_wrapper(std::wstring&& str);
		string_wrapper(std::u8string&& str);

		template <typename C>
		string_wrapper(const std::basic_string_view<C>& str)
			: string_wrapper(std::basic_string<C>(str._Unchecked_begin( ), str._Unchecked_end( )))
		{
		}

		template <typename T, size_t N>
		string_wrapper(const T (&str)[N])
			: string_wrapper(std::basic_string<T>(str, str + (N - 1)))
		{
		}

		operator value_type() const;
		value_type imgui() const;
	};

	//deprecated!!!
	class perfect_string
	{
	public:
		perfect_string(const string_wrapper& wrstr);
		perfect_string(string_wrapper&& wrstr);
		perfect_string(string_wrapper::const_type str);

		operator string_wrapper::value_type() const;

		size_t chars_count() const;
		size_t chars_capacity() const;

	private:
		using string_wrapper_ref = std::reference_wrapper<const string_wrapper>;
		using holder_type = std::variant<
			string_wrapper_ref
		  , string_wrapper
		  , string_wrapper::value_type
		>;

		holder_type holder_;
		size_t      chars_count_, chars_capacity_;
	};

	//----

	//todo: template this, separate with render function
	//return str(const string&), view(string_view)
	class cached_text
	{
		template <typename T>
		struct size_checker : std::bool_constant<sizeof(ImWchar) == sizeof(T)>
		{
			using type = T;
		};

		template <typename ...Ts>
		using char_type_helper = std::disjunction<size_checker<Ts>...>;

	public:
		using char_type = char_type_helper<char8_t, char16_t, char32_t>::type;

		void set_font(ImFont* new_font);

		void set_label(const std::string_view& str);
		void set_label(const std::u8string_view& str);
		void set_label(const std::wstring_view& str);
		void set_label(const std::u16string_view& str);
		void set_label(const std::u32string_view& str);

		void set_label(std::string&& str);
		void set_label(std::u8string&& str);
		void set_label(std::wstring&& str);
		void set_label(std::u16string&& str);
		void set_label(std::u32string&& str);

		template <typename T, size_t N>
		void set_label(const T (&str)[N])
		{
			set_label((std::basic_string_view<T>(str, str + (N - 1))));
		}

	private:
		void update();

	public:
		void render(ImDrawList* draw_list, ImVec2 pos, ImU32 color) const;

	private:
		//todo: invisible chars ignored
		std::basic_string<char_type> label_;
		ImVec2                       label_size_;

		//std::vector<ImFontGlyph> glyphs_;//ImFontGlyph invalid after efery atlas build, so it uselles
		size_t visible_glyphs_count_ = 0;

		ImFont* font_ = 0;

	public:
		const ImVec2& get_label_size() const { return label_size_; }
		ImFont*       get_font() const { return font_; }
	};
}
