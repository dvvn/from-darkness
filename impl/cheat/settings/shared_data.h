#pragma once

#include <stdint.h>

namespace std
{
	template <class E, class Tr>
	class basic_string_view;

	template <class E, class Tr, class A>
	class basic_string;

	template <typename E>
	struct char_traits;

	template <class T>
	class allocator;

	template <class T1, class T2>
	struct pair;

	template <class T>
	struct less;

	template <class Key, class T, class Fn /*= less<_Kty>*/, class A /*= allocator<pair<const Key, T>>*/>
	class map;

	template <class T, class A /*= allocator<T>*/>
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

namespace cheat::settings
{
	class shared_data
	{
	public:
		virtual ~shared_data( ) = default;

		/*using ostream = std::basic_ostream<char, std::char_traits<char>>;
		using istream = std::basic_istream<char, std::char_traits<char>>;

		friend ostream& operator<<(ostream& stream, const shared_data& shared_data);
		friend istream& operator>>(istream& stream, shared_data& shared_data);*/

		using json = nlohmann::basic_json
		<
			std::map,
			std::vector,
			std::basic_string<char, std::char_traits<char>, std::allocator<char>>, //wstring unsupported
			bool,
			intptr_t,
			size_t,
			double,
			std::allocator,
			nlohmann::adl_serializer,
			std::vector<uint8_t, std::allocator<uint8_t>>
		>;

		virtual void save(json& in) const =0;
		virtual void load(const json& out) =0;

		/*protected:
			virtual void save(ostream& stream) const =0;
			virtual void load(const istream& stream) =0;*/
	};
}
