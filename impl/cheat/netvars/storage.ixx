module;

//#include <nlohmann/json.hpp>
//#include <nlohmann/ordered_map.hpp>

#include <nstd/core.h>

#include <vector>
#include <string>
#include <sstream>

export module cheat.netvars:storage;
import :basic_storage;
import cheat.csgo.structs.ClientClass;

#if 0
template <class Key, class Value, class IgnoredLess = std::less<Key>    //
, class Allocator = std::allocator<std::pair<const Key, Value>> //
, class Base = nlohmann::ordered_map<Key, Value, IgnoredLess, Allocator>
>
struct ordered_map_json : Base
{
	using typename Base::key_type;
	using typename Base::mapped_type;

	using typename Base::Container;

	using typename Base::iterator;
	using typename Base::const_iterator;
	using typename Base::size_type;
	using typename Base::value_type;

	template <std::equality_comparable_with<Key> Key2, typename ...Args>
		requires(std::constructible_from<Key, Key2>)
	std::pair<iterator, bool> emplace(Key2&& key, Args&&...args)
	{
		auto found = this->find(key);
		if (found != this->end( ))
			return {found, false};

		if constexpr (sizeof...(Args) == 0)
		{
			static_assert(std::default_initializable<Value>, "Unable to construct empty mapped type");
			this->emplace_back(std::forward<Key2>(key), Value( ));
		}
		else
		{
			this->emplace_back(std::forward<Key2>(key), std::forward<Args>(args)...);
		}
		return {std::prev(this->end( )), true};
	}

	template <std::equality_comparable_with<Key> Key2>
	iterator find(const Key2& key)
	{
		auto begin = this->begin( );
		auto end = this->end( );
		for (auto itr = begin; itr != end; ++itr)
		{
			if (itr->first == key)
				return itr;
		}
		return end;
	}

	template <std::equality_comparable_with<Key> Key2>
	const_iterator find(const Key2& key) const
	{
		return std::_Const_cast(this)->find(key);
	}

	//----

	using _Char_type = Key::value_type;

	template<typename ...Args>
	std::pair<iterator, bool> emplace(const _Char_type* key, Args&&...) = delete;

	iterator find(const _Char_type* key) = delete;
	const_iterator find(const _Char_type* key) const = delete;
};
#endif

export namespace cheat::netvars
{
	struct logs_data
	{
		~logs_data( );

		std::wstring dir = NSTD_STRINGIZE_RAW_WIDE(NSTD_CONCAT(VS_SolutionDir, \.dumps\netvars\));

		struct
		{
			std::wstring name;
			std::wstring extension = L".json";
		}file;

		size_t indent = 4;
		char filler = ' ';

		std::ostringstream buff;
	};

	struct classes_data
	{
		~classes_data( );

		std::wstring dir = NSTD_STRINGIZE_RAW_WIDE(NSTD_CONCAT(VS_SolutionDir, \impl\cheat\csgo\interfaces_custom\));

		struct file_info
		{
			std::wstring name;
			std::ostringstream buff;
		};

		std::vector<file_info> files;
	};

	class storage : public basic_storage
	{
	public:
		void iterate_client_class(csgo::ClientClass* const root_class) noexcept;
		void iterate_datamap(csgo::datamap_t* const root_map) noexcept;
		void store_handmade_netvars( ) noexcept;

		void log_netvars(logs_data& data) noexcept;
		void generate_classes(classes_data& data) noexcept;
	};

	//using storage=nlohmann::basic_json<ordered_map_json, std::vector, std::string, bool, std::make_signed_t<size_t>, size_t, float>;
	//using storage = nlohmann::json;
}
