module;

export module cheat.csgo.awaiter;
export import cheat.service;

export namespace cheat
{
	class csgo_awaiter final : public dynamic_service<csgo_awaiter>
	{
		void construct( ) noexcept override;
		bool load( ) noexcept override;
	};
}
