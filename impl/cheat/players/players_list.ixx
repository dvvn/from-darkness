module;

#include <vector>

export module cheat.players:list;
export import :player;

namespace cheat
{
	template<typename T>
	decltype(auto) _At(T& storage, size_t index)
	{
		return storage[index - 1];
	}

	class players_list_storage
	{
	public:

		void resize(size_t count)
		{
			storage_.resize(count);
		}

		player& operator[](size_t index)
		{
			return _At(storage_, index);
		}
		const player& operator[](size_t index)const
		{
			return _At(storage_, index);
		}

		auto begin( )
		{
			return storage_.begin( );
		}
		auto begin( )const
		{
			return storage_.begin( );
		}

		auto end( )
		{
			return storage_.end( );
		}
		auto end( )const
		{
			return storage_.end( );
		}

	private:
		std::vector<player> storage_;
	};

	export struct players_list final : dynamic_service<players_list>
	{
		players_list( );
		~players_list( ) override;

		void update( );

		//const detail::players_filter& filter(const players_filter_flags& flags);

	protected:
		void construct( ) noexcept override;

	private:
		players_list_storage storage_;
		//nstd::unordered_set<detail::players_filter> filter_cache__;
	};
}
