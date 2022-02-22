module;

#include <atomic>

export module cheat.csgo.awaiter;
export import cheat.service;

export namespace cheat
{
	class csgo_awaiter final : public dynamic_service<csgo_awaiter>
	{
		std::shared_ptr<std::atomic_bool> destroyed_;

	protected:
		void construct( ) noexcept override;
		bool load( ) noexcept override;

	public:
		~csgo_awaiter( )override;

		bool game_loaded_before = false;
	};

}
