#pragma once

namespace std
{
	template <class E, class Tr>
	class basic_string_view;

	template <class E, class Tr, class A>
	class basic_string;

	template <typename E>
	struct char_traits;

	template <class _Ty>
	class allocator;

	using wstring_view = basic_string_view<wchar_t, char_traits<wchar_t>>;

	using wstring = basic_string<wchar_t, char_traits<wchar_t>, allocator<wchar_t>>;
	using string = basic_string<char, char_traits<char>, allocator<char>>;

	template <class _Ty1, class _Ty2>
	struct pair;

	template <class _Ty>
	struct less;

	template <class _Kty, class _Ty, class _Pr /*= less<_Kty>*/, class _Alloc /*= allocator<pair<const _Kty, _Ty>>*/>
	class map;

	template <class _Ty, class _Alloc /*= allocator<_Ty>*/>
	class vector;
}

namespace nlohmann
{
	/*!
	@brief default JSONSerializer template argument
	
	This serializer ignores the template arguments and uses ADL
	([argument-dependent lookup](https://en.cppreference.com/w/cpp/language/adl))
	for serialization.
	*/
	template <typename T /*= void*/, typename SFINAE /*= void*/>
	struct adl_serializer;

	template <template<typename U, typename V, typename... Args> class ObjectType /*= std::map*/,
			  template<typename U, typename... Args> class ArrayType /*= std::vector*/,
			  class StringType/* = std::string*/,
			  class BooleanType /*= bool*/,
			  class NumberIntegerType /*= std::int64_t*/,
			  class NumberUnsignedType /*= std::uint64_t*/,
			  class NumberFloatType /*= double*/,
			  template<typename U> class AllocatorType /*= std::allocator*/,
			  template<typename T, typename SFINAE /*= void*/> class JSONSerializer /*= adl_serializer*/,
			  class BinaryType /*= std::vector<std::uint8_t, std::allocator<std::uint8_t>>*/>
	class basic_json;
}

namespace cheat
{
	class settings_shared
	{
	public:
		settings_shared( );
		virtual ~settings_shared( );

		/*using ostream = std::basic_ostream<char, std::char_traits<char>>;
		using istream = std::basic_istream<char, std::char_traits<char>>;

		friend ostream& operator<<(ostream& stream, const settings_shared& shared);
		friend istream& operator>>(istream& stream, settings_shared& shared);*/

		using json = nlohmann::basic_json
		<
			std::map,
			std::vector,
			std::string,//wstring unsupported
			bool,
			int,
			unsigned int,
			double,
			std::allocator,
			nlohmann::adl_serializer,
			std::vector<unsigned char, std::allocator<unsigned char>>
		>;

		virtual void save(json& in) const =0;
		virtual void load(const json& out) =0;

		virtual std::wstring_view title( ) const =0;

		/*protected:
			virtual void save(ostream& stream) const =0;
			virtual void load(const istream& stream) =0;*/
	};
}
