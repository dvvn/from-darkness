#pragma once

#if !defined(IMGUI_HAS_IMSTR) || !IMGUI_HAS_IMSTR
#define CHEAT_GUI_HAS_IMGUI_STRV 0
#else
#define CHEAT_GUI_HAS_IMGUI_STRV 1
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
		std::weak_ordering operator<=>(const string_wrapper& other) const;

		operator utl::wstring_view( ) const;
		operator utl::string_view( ) const;
		operator value_type( ) const;

		utl::wstring_view raw( ) const;
		utl::string_view multibyte( ) const;
		value_type imgui( ) const;

	private:
		void Set_imgui_str_( );

		raw_type raw__;
		multibyte_type multibyte__;
		value_type imgui__;
	};

	string_wrapper::value_type _Get_imgui_str(const utl::string_view& str);

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

	class prefect_string
	{
	public:
		using ref_or_direct_type = std::conditional_t<std::is_class_v<string_wrapper::value_type>, const string_wrapper::value_type&, string_wrapper::value_type>;

		prefect_string(ref_or_direct_type str);
		prefect_string(const utl::string_view& str);
		prefect_string(const utl::wstring_view& str);
		prefect_string(utl::wstring&& str);
		prefect_string(const string_wrapper& str);

		operator string_wrapper::value_type( ) const;

		size_t size( ) const;

	private:
		using string_wrapper_ref = utl::reference_wrapper<const string_wrapper>;
		using holder_type = utl::variant
		<
			string_wrapper_ref,
			string_wrapper,
			string_wrapper::value_type
		>;

		holder_type holder__;
#if !defined(IMGUI_HAS_IMSTR) || !IMGUI_HAS_IMSTR
		mutable utl::optional<size_t> size__;
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
			return invoke(hash<string_view>( ), (str));
		}
	};

_STD_END
