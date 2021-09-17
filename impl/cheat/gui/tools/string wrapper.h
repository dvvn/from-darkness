#pragma once

#include <optional>
#include <string>
#include <variant>

#include <imgui.h>

namespace cheat::gui::tools
{
	namespace detail
	{
		struct string_wrapper_multibyte_view
		{
		};
	}

	class string_wrapper
	{
	public:
#ifdef IMGUI_HAS_IMSTR
	using value_type = ImStrv;
#else
		using value_type = /*const*/ char*;
#endif
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
			: string_wrapper(std::basic_string_view<T>(str, str + (N - 1)))
		{
		}

		~string_wrapper( );
		string_wrapper( );

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
		union
		{
			// ReSharper disable CppInconsistentNaming
			std::u8string multibyte_real_;
			std::string   multibyte_;
			// ReSharper restore CppInconsistentNaming
		};

		std::wstring raw_;
	};

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
