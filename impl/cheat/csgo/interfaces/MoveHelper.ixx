
export module cheat.csgo.interfaces.MoveHelper;
import nstd.one_instance;

export namespace cheat::csgo
{
	class IClientEntity;

	class IMoveHelper :public nstd::one_instance<IMoveHelper*>
	{
	public:
		virtual	void _vpad( ) = 0;
		virtual void SetHost(IClientEntity* host) = 0;
	};
}