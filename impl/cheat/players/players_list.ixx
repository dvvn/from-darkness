module;

#include <vector>

export module cheat.players:list;
export import :player;

export namespace cheat
{
	struct players_list final : dynamic_service<players_list>
	{
		players_list( );
		~players_list( ) override;

		void update( );

		//const detail::players_filter& filter(const players_filter_flags& flags);

	protected:
		void construct( ) noexcept override;

	private:
		std::vector<player> storage_;
		//nstd::unordered_set<detail::players_filter> filter_cache__;
	};
}
