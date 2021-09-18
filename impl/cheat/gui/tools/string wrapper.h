#pragma once

#include <optional>
#include <string>
#include <variant>

#include <imgui.h>

namespace cheat::gui::tools
{
	namespace detail
	{
		class string_wrapper_multibyte
		{
		public:
			auto operator<=>(const string_wrapper_multibyte&) const = default;

			operator std::string_view( ) const { return multibyte_; }
			std::string_view multibyte( ) const { return multibyte_; }

		protected:
			// ReSharper disable once CppInconsistentNaming
			std::string multibyte_;
		};

		class string_wrapper_raw
		{
		public:
			operator std::wstring_view( ) const { return raw_; }
			std::wstring_view raw( ) const { return raw_; }

		protected:
			// ReSharper disable once CppInconsistentNaming
			std::wstring raw_;
		};
	}

	class string_wrapper: public detail::string_wrapper_multibyte, public detail::string_wrapper_raw
	{
	public:
#ifdef IMGUI_HAS_IMSTR
		using value_type = ImStrv;
		using const_type= const ImStrv&
#else
		using value_type = /*const*/ char*;
		using const_type = const char*;
#endif
		string_wrapper( ) = default;

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

		operator value_type( ) const;
		value_type imgui( ) const;
	};

	class perfect_string
	{
	public:
		perfect_string(const string_wrapper& wrstr);
		perfect_string(string_wrapper&& wrstr);
		perfect_string(string_wrapper::const_type str);

		operator string_wrapper::value_type( ) const;

		size_t chars_count( ) const;

	private:
		using string_wrapper_ref = std::reference_wrapper<const string_wrapper>;
		using holder_type = std::variant
		<
			string_wrapper_ref
		  , string_wrapper
		  , string_wrapper::value_type
		>;

		holder_type holder_;

#ifndef IMGUI_HAS_IMSTR
		size_t chars_count_;
#endif
	};
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
