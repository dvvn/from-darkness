#pragma once

#include <optional>
#include <string>
#include <variant>

#include <imgui.h>
#ifdef IMGUI_HAS_IMSTR
#define CHEAT_GUI_HAS_IMGUI_STRV 1
#else
#define CHEAT_GUI_HAS_IMGUI_STRV 0
#endif

namespace cheat::gui::tools
{
	class string_wrapper
	{
	public:
#if !CHEAT_GUI_HAS_IMGUI_STRV
		using value_type = /*const*/ char*;
#else
		using value_type = ImStrv;
#endif
		using raw_type = std::wstring;
		using multibyte_type = std::string;

		string_wrapper(raw_type&& str);
		string_wrapper(multibyte_type&& str);

		template <typename T, size_t N>
		string_wrapper(const T (&str)[N])
			: string_wrapper(std::basic_string<T>(str, str + (N - 1)))
		{
		}

		template <typename E, typename Tr>
		string_wrapper(const std::basic_string_view<E, Tr>& str)
			: string_wrapper(std::basic_string<E, Tr>(str._Unchecked_begin( ), str._Unchecked_end( )))
		{
		}

		string_wrapper( ) = default;

		string_wrapper(string_wrapper&& other) noexcept;
		string_wrapper& operator=(string_wrapper&& other) noexcept;

		string_wrapper(const string_wrapper& other) noexcept;
		string_wrapper& operator=(const string_wrapper& other) noexcept;

		bool operator==(const string_wrapper& other) const;
		bool operator!=(const string_wrapper& other) const;

		std::weak_ordering operator<=>(const string_wrapper& other) const;

		operator std::wstring_view( ) const;
		operator std::string_view( ) const;
		operator value_type( ) const;

		std::wstring_view raw( ) const;
		std::string_view  multibyte( ) const;
		value_type        imgui( ) const;

	private:
		void Set_imgui_str_( );

		raw_type       raw_;
		multibyte_type multibyte_;
		value_type     imgui_;
	};

	string_wrapper::value_type _Get_imgui_str(const std::string_view& str);

	/*
	class string_wrapper_base: public string_wrapper
	{
	public:
		string_wrapper_base(string_wrapper&& other)
			: string_wrapper(std::move(other))
		{
		}

		string_wrapper& name( )
		{
			return *this;
		}

		const string_wrapper& name( ) const
		{
			return *this;
		}
	};*/

	/*class string_wrapper_abstract
	{
	public:
		string_wrapper_abstract( );

		operator const string_wrapper&( ) const;
		const string_wrapper& get( ) const;

		void init(string_wrapper&& name);
		void init(const string_wrapper& name);
		void init(const string_wrapper* name);

	private:
		std::variant<string_wrapper,
					 std::reference_wrapper<const string_wrapper>> name_;
	};*/

	class perfect_string
	{
	public:
		using ref_or_direct_type = std::conditional_t
		<
			std::is_class_v<string_wrapper::value_type>,
			const string_wrapper::value_type&, string_wrapper::value_type
		>;

		perfect_string(ref_or_direct_type str);
		perfect_string(const std::string_view& str);
		perfect_string(const std::wstring_view& str);
		perfect_string(std::wstring&& str);
		perfect_string(const string_wrapper& str);

		operator string_wrapper::value_type( ) const;

		size_t size( ) const;

		bool operator==(const string_wrapper& wrapped) const;
		bool operator!=(const string_wrapper& wrapped) const;

	private:
		using string_wrapper_ref = std::reference_wrapper<const string_wrapper>;
		using holder_type = std::variant
		<
			string_wrapper_ref,
			string_wrapper,
			string_wrapper::value_type
		>;

		holder_type holder_;
#if !CHEAT_GUI_HAS_IMGUI_STRV
		mutable std::optional<size_t> size_;
#endif
	};

	bool operator==(const string_wrapper& a, const perfect_string& b);
	bool operator!=(const string_wrapper& a, const perfect_string& b);
}

_STD_BEGIN
	// ReSharper disable once CppInconsistentNaming
	using _Imgui_string = cheat::gui::tools::string_wrapper;

	template <std::derived_from<_Imgui_string> T>
	struct hash<T>
	{
		_NODISCARD size_t operator()(const T& str) const noexcept
		{
			return std::invoke(hash<string_view>( ), str);
		}
	};

_STD_END
