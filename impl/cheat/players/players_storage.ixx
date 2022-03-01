module;

export module cheat.players;
export import cheat.players.player;

export namespace cheat::players
{
	player* begin( );
	player* end( );
	void update( );
}
