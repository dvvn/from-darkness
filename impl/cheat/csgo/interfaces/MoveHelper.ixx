
export module cheat.csgo.interfaces.MoveHelper;

export namespace cheat::csgo
{
	class IClientEntity;

	class IMoveHelper
	{
	public:
		virtual	void _vpad( ) = 0;
		virtual void SetHost(IClientEntity* host) = 0;
	};
}