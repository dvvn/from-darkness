#pragma once

namespace cheat::gui::imgui
{
	class string_wrapper
	{
	public:
#if !defined(IMGUI_HAS_IMSTR) || !IMGUI_HAS_IMSTR
		using value_type = /*const*/ char*;
#else
		using value_type = ImStrv;
#endif
		using raw_type = utl::wstring;
		using multibyte_type = utl::string;

		string_wrapper(raw_type&& str);
		string_wrapper(multibyte_type&& str);

		template <typename T, size_t N>
		string_wrapper(const T (&str)[N]) : string_wrapper(utl::basic_string<T>(str, N - 1))
		{
		}

		string_wrapper( ) = default;

		string_wrapper(string_wrapper&& other) noexcept;
		string_wrapper& operator=(string_wrapper&& other) noexcept;

		string_wrapper(const string_wrapper& other) noexcept;
		string_wrapper& operator=(const string_wrapper& other) noexcept;

		bool operator==(const string_wrapper& other) const;
		bool operator!=(const string_wrapper& other) const;

		operator utl::wstring_view( ) const;
		operator utl::string_view( ) const;
		operator value_type( ) const;

		utl::wstring_view raw( ) const;
		utl::string_view  multibyte( ) const;
		value_type        imgui( ) const;

	private:
		void Set_imgui_str_( );

		raw_type       raw__;
		multibyte_type multibyte__;
		value_type     imgui__;
	};

	class string_wrapper_base: public string_wrapper
	{
	public:
		string_wrapper_base(string_wrapper&& other) : string_wrapper(utl::move(other))
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
	};

	class string_wrapper_abstract
	{
	public:
		string_wrapper_abstract( );

		operator const string_wrapper&( ) const;
		const string_wrapper& get( ) const;

		void init(string_wrapper&& name);
		void init(const string_wrapper& name);

		template <class T>
		T* init(T* obj)
		{
			init(obj->name( ));
			return obj;
		}

	private:
		utl::variant<string_wrapper,
					 utl::reference_wrapper<const string_wrapper>> name__;
	};
}

_STD_BEGIN
	// ReSharper disable once CppInconsistentNaming
	using _Imgui_string = cheat::gui::imgui::string_wrapper;

	template < >
	struct hash<_Imgui_string>: hash<wstring_view>
	{
		_NODISCARD size_t operator()(const _Imgui_string& str) const noexcept
		{
			return invoke(*static_cast<const hash<wstring_view>*>(this), (str.raw( )));
		}
	};

#if 0
template < >
struct equal_to<_Imgui_string> : equal_to<string>
{
	// ReSharper disable once CppRedundantInlineSpecifier
	_NODISCARD _CONSTEXPR20_CONTAINER auto operator()(const _Imgui_string& left, const _Imgui_string& right) const -> bool
	{
		return invoke(*static_cast<const equal_to<string>*>(this), left.multibyte, right.multibyte);
	}
};
#endif

_STD_END
